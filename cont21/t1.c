#include <openssl/evp.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv) {
  // Получим входные данные
  char* password = argv[1];
  char trash[8];
  read(0, trash, 8);
  char salt[8];
  read(0, salt, 8);

  // Создание контекста
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  EVP_CIPHER_CTX_init(ctx);

  // Инициализация
  unsigned char key[EVP_CIPHER_key_length(EVP_aes_256_cbc())];
  unsigned char iv[EVP_CIPHER_iv_length(EVP_aes_256_cbc())];
  EVP_BytesToKey(EVP_aes_256_cbc(),  // алгоритм шифрования
                 EVP_sha256(),  // хеш-функция
                 (const unsigned char *)salt,  // соль
                 (const unsigned char *)password,  // пароль
                 (int)strlen(password),  // длина пароля
                 1,  // количество итераций хеширования
                 key,  // буфер для ключа
                 iv  // буфер для начального вектора
                 );
  EVP_DecryptInit(ctx,                  // контекст для хранения состояния
                  EVP_aes_256_cbc(),    // алгоритм шифрования
                  key,                  // ключ нужного размера
                  iv                    // начальное значение нужного размера
                  );

  // Добавление очередной порции данных
  char encrypted_data[4096] = {};
  char decrypted_data[4096] = {};
  int encrypted_data_size = 0;
  int decrypted_data_size = 0;
  while ((encrypted_data_size = (int)read(0, encrypted_data, sizeof(encrypted_data))) > 0) {
    EVP_DecryptUpdate(ctx,  // контекст
                      (unsigned char*)decrypted_data, &decrypted_data_size,  // буфер и размер для записи расшифрованных данных
                      (const unsigned char *)encrypted_data, encrypted_data_size  // буфер и размер для доступа к зашифрованным данным
                      );
    write(1, decrypted_data, decrypted_data_size);
  }

  // Финализация и освобожление контекста
  int final_decrypted_data_size = 0;
  EVP_DecryptFinal(ctx, (unsigned char*)decrypted_data, &final_decrypted_data_size);
  write(1, decrypted_data, final_decrypted_data_size);
  EVP_CIPHER_CTX_cleanup(ctx);
  return 0;
}
