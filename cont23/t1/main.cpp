#include "interfaces.h"
#include <dlfcn.h>
#include <algorithm>
#include <unistd.h>

// Подгружаемая функция (конструктор класса)
typedef void (*function_t)(void*);

// Структура с данными о конструкторе
struct ClassImpl {
  function_t constructor = nullptr;
  void* lib_ptr = nullptr;
};

// Структура с данными об указателях на подгруженные функции и об ошибке
struct ClassLoaderImpl {
  std::vector<AbstractClass*> class_ptr_vec;
  ClassLoaderError err = ClassLoaderError::NoError;
};

////////////////////////////////////////////////////////////////////////////////
///                         Additional functions                             ///
////////////////////////////////////////////////////////////////////////////////

// Получает тег функции-конструктора в машинном коде
std::string GetName(const std::string& full_name) {
  std::string func_name;
  /**
    The "_ZN" prefix is a token that designates C++
    full_name mangling, followed by namespace the function belongs,
    then function names and argument types,
    then templates, if any.
  */
  func_name += "_ZN";
  size_t cur_delim_pos = 0;  // Позиция текущего ::
  size_t next_delim_pos = full_name.find("::", cur_delim_pos);  // Позиция следующего ::

  while (next_delim_pos != std::string::npos) {
    size_t namespace_size = next_delim_pos - cur_delim_pos;
    func_name += std::to_string(namespace_size);
    func_name += full_name.substr(cur_delim_pos, namespace_size);
    cur_delim_pos = next_delim_pos + 2;
    next_delim_pos = full_name.find("::", cur_delim_pos);
  }
  func_name += std::to_string(full_name.length() - cur_delim_pos);
  func_name += full_name.substr(cur_delim_pos);
  func_name += "C1Ev";
  return func_name;
}

// Получает путь до библиотеки
std::string ProcessPath(const std::string& name) {
  std::string path_part = name;
  std::replace(path_part.begin(), path_part.end(), ':', '/');
  char* class_path_c_str = std::getenv("CLASSPATH");
  std::string class_path = class_path_c_str;
  std::string lib_path = class_path + "/" + path_part + ".so";
  return lib_path;
}

////////////////////////////////////////////////////////////////////////////////
///                     Implementation of methods                            ///
////////////////////////////////////////////////////////////////////////////////

// Вызывает конструктор класса
void* AbstractClass::newInstanceWithSize(size_t sizeofClass) {
  void* obj = new char[sizeofClass];
  pImpl->constructor(obj);
  return obj;
}

AbstractClass::AbstractClass() : pImpl(new ClassImpl()) { }

AbstractClass::~AbstractClass() {
  if (pImpl->lib_ptr != nullptr) {
    dlclose(pImpl->lib_ptr);
  }
  delete pImpl;
}

ClassLoader::ClassLoader() : pImpl(new ClassLoaderImpl()) { }

ClassLoader::~ClassLoader() {
  for (auto i : pImpl->class_ptr_vec) {
    delete i;
  }
  delete pImpl;
}

AbstractClass* ClassLoader::loadClass(const std::string& fullyQualifiedName) {
  // Получаю путь до библиотеки
  std::string path = ProcessPath(fullyQualifiedName);
  if (access(path.c_str(), F_OK) != 0) {
    pImpl->err = ClassLoaderError::FileNotFound;
    return nullptr;
  }
  // Загружаю библиотеку
  void* lib = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (!lib) {
    pImpl->err = ClassLoaderError::LibraryLoadError;
    return nullptr;
  }
  // Получаю указатель на функцию-конструктор
  std::string new_name = GetName(fullyQualifiedName);
  function_t func =
      reinterpret_cast<function_t>(dlsym(lib, new_name.c_str()));
  if (func == nullptr) {
    pImpl->err = ClassLoaderError::NoClassInLibrary;
    return nullptr;
  }
  // Создаю объект класса и кладу указатель на выделенную память в вектор
  pImpl->class_ptr_vec.push_back(new AbstractClass());
  pImpl->class_ptr_vec.back()->pImpl->lib_ptr = lib;
  pImpl->class_ptr_vec.back()->pImpl->constructor = func;
  // Обнуляю ошибку и возвращаю указатель на объект класса
  pImpl->err = ClassLoaderError::NoError;
  return pImpl->class_ptr_vec.back();
}

ClassLoaderError ClassLoader::lastError() const { return pImpl->err; }
