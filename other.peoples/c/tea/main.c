#include <stdio.h>
#include "tea.h"
#include "xtea.h"
#include "xxtea.h"

static uint32_t data[2] = {0x1234,0x5678};  
static uint32_t key[4]  = {0x1,0x2,0x3,0x4};

int main(void){
        printf("/*tea*/\n");
        printf("data = {0x%X, 0x%X};\n", data[0], data[1]);
        printf("key  = {0x%X, 0x%X, 0x%X, 0x%X};\n", key[0], key[1], key[2], key[3]);
        tea_encrypt(data, key);
        printf("encrypted = {0x%X, 0x%X};\n", data[0], data[1]);
        tea_decrypt(data, key);
        printf("decrypted = {0x%X, 0x%X};\n", data[0], data[1]);

        printf("/*xtea*/\n");
        printf("data = {0x%X, 0x%X};\n", data[0], data[1]);
        printf("key  = {0x%X, 0x%X, 0x%X, 0x%X};\n", key[0], key[1], key[2], key[3]);
        xtea_encrypt(64, data, key);
        printf("encrypted = {0x%X, 0x%X};\n", data[0], data[1]);
        xtea_decrypt(64, data, key);
        printf("decrypted = {0x%X, 0x%X};\n", data[0], data[1]);

        printf("/*xxtea*/\n");
        printf("data = {0x%X, 0x%X};\n", data[0], data[1]);
        printf("key  = {0x%X, 0x%X, 0x%X, 0x%X};\n", key[0], key[1], key[2], key[3]);
        xxtea_encrypt(2, data, key);
        printf("encrypted = {0x%X, 0x%X};\n", data[0], data[1]);
        xxtea_decrypt(2, data, key);
        printf("decrypted = {0x%X, 0x%X};\n", data[0], data[1]);





        return 0;
}
