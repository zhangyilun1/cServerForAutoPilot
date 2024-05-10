#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include "md5.h"
#include <sys/stat.h>
using namespace std;
#define _CRT_SECURE_NO_WARNINGS
// MD5 Round Constants
const uint32_t MD5_K[] = {
    0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476
};

// MD5 Shift Amounts
const uint32_t MD5_S[] = {
    7, 12, 17, 22,
    5, 9, 14, 20,
    4, 11, 16, 23,
    6, 10, 15, 21
};

// MD5 Initialization Values (A, B, C, D)
const uint32_t MD5_IV[] = {
    0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476
};

// MD5 Padding
const uint8_t MD5_PADDING[64] = {0x80};

// Helper Functions
inline uint32_t leftRotate(uint32_t value, uint32_t shift) {
    return (value << shift) | (value >> (32 - shift));
}

void md5Hash(const uint8_t *input, size_t length, uint32_t hash[4]) {
    uint32_t a = MD5_IV[0];
    uint32_t b = MD5_IV[1];
    uint32_t c = MD5_IV[2];
    uint32_t d = MD5_IV[3];

    // Main loop
    for (size_t i = 0; i < length; i += 64) {
        uint32_t words[16];
        for (size_t j = 0; j < 16; ++j) {
            words[j] = input[i + j * 4] | (input[i + j * 4 + 1] << 8) |
                       (input[i + j * 4 + 2] << 16) | (input[i + j * 4 + 3] << 24);
        }

        uint32_t aa = a;
        uint32_t bb = b;
        uint32_t cc = c;
        uint32_t dd = d;

        for (size_t j = 0; j < 64; ++j) {
            uint32_t f, g;

            if (j < 16) {
                f = (b & c) | ((~b) & d);
                g = j;
            } else if (j < 32) {
                f = (d & b) | ((~d) & c);
                g = (5 * j + 1) % 16;
            } else if (j < 48) {
                f = b ^ c ^ d;
                g = (3 * j + 5) % 16;
            } else {
                f = c ^ (b | (~d));
                g = (7 * j) % 16;
            }

            uint32_t temp = d;
            d = c;
            c = b;
            b = b + leftRotate((a + f + MD5_K[j] + words[g]), MD5_S[j]);
            a = temp;
        }

        a += aa;
        b += bb;
        c += cc;
        d += dd;
    }

    hash[0] = a;
    hash[1] = b;
    hash[2] = c;
    hash[3] = d;
}

std::string byteArrayToHex(const std::vector<uint8_t> &bytes) {
    std::stringstream ss;
    for (const uint8_t byte : bytes) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return ss.str();
}

std::string calculateMD5(const std::string &filename) {
    struct stat fileInfo; 
    if (stat(filename.c_str(), &fileInfo) != 0) { 
        std::cerr << "无法获取文件信息" << std::endl; 
        return ""; 
    } 
    int fileSize = fileInfo.st_size; // 获取文件大小 
    int mod = fileSize%64;
    int reallen = fileSize-mod;

    // std::ifstream file(filename, std::ios::binary);
    // if (!file || !file.is_open()) {
    //     std::cerr << "Error opening file: " << filename << std::endl;
    //     return "";
    // }

    std::vector<uint8_t> input;
    // char buffer[64];
    // while (file.read(buffer, sizeof(buffer))) {
    //     for (size_t i = 0; i < file.gcount(); ++i) {
    //         input.push_back(buffer[i]);
    //     }
    // }
    
    char *buf = new char[reallen];
    if (!buf) return "";

    FILE *fp;  
   // fp = fopen(filename.c_str(), "rb");  
   // fopen_s(&file, filename.c_str(), "mode")
    if (fopen_s(&fp, filename.c_str(), "rb")!=0) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }

    int numRead = fread(buf, sizeof(char), reallen, fp);  
    if (numRead != reallen) {
        std::cerr << "read num: " << numRead << " file size: " << fileSize << std::endl;
    }
    fclose(fp);
    //file.read(buf, reallen);

    for (int i=0; i<reallen; i++) {
        input.push_back(buf[i]);
    }
    delete []buf;
    // Add padding
    size_t originalLength = input.size();
    input.push_back(0x80);
    while ((input.size() % 64) != 56) {
        input.push_back(0);
    }

    // Append original length in bits
    uint64_t bitLength = originalLength * 8;
    for (size_t i = 0; i < 8; ++i) {
        input.push_back(static_cast<uint8_t>(bitLength >> (i * 8)));
    }

    uint32_t hash[4];
    md5Hash(input.data(), input.size(), hash);

    std::vector<uint8_t> hashBytes;
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            hashBytes.push_back(static_cast<uint8_t>(hash[i] >> (j * 8)));
        }
    }

    return byteArrayToHex(hashBytes);
}




std::string calculateMD5byOpenSSL(const std::string &filename) {
    //FILE *file = fopen(filename.c_str(), "rb");
    FILE* file;
   // if (fopen_s(&file, filename.c_str(), "mode") == 0)
    if (fopen_s(&file, filename.c_str(), "rb") != 0) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }

    EVP_MD_CTX* mdctx;
    unsigned char* md5_digest;
    unsigned int md5_digest_len = EVP_MD_size(EVP_md5());
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);

    //MD5_CTX md5Context;
    //MD5_Init(&md5Context);

    unsigned char buffer[1024];
    size_t bytesRead;
    std::cout << sizeof(buffer) << std::endl;
    // int i = 1;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        
        // std::cout << i << "  bytesRead  : " << bytesRead << std::endl;
       // MD5_Update(&md5Context, buffer, bytesRead);
        EVP_DigestUpdate(mdctx, buffer, bytesRead);
        // i++;
    }

    fclose(file);

    //unsigned char md5Digest[MD5_DIGEST_LENGTH];
   // MD5_Final(md5Digest, &md5Context);

    md5_digest = (unsigned char*)OPENSSL_malloc(md5_digest_len);
    EVP_DigestFinal_ex(mdctx, md5_digest, &md5_digest_len);
    EVP_MD_CTX_free(mdctx);



    std::stringstream md5Stream;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        md5Stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(md5_digest[i]);
    }

    return md5Stream.str();
}


std::string calculateMD5(char * buf, int len) {
    std::vector<uint8_t> input;
    int mod = len%64;
    int reallen = len-mod;
    for (int i=0; i<reallen; i++) {
        input.push_back(buf[i]);
    }

    std::cout << "input size:" << input.size() << std::endl;




    // for (int i=0; i<10; i++) {
    //     std::cout << std::hex << (int)(input[i]) << " ";
    // }
    // std::cout << std::endl;

    // for (int i=41016; i<41027; i++) {
    //     std::cout << std::hex << (int)(input[i]) << " ";
    // }
    // std::cout << std::endl;




    // Add padding
    size_t originalLength = input.size();
    input.push_back(0x80);
    while ((input.size() % 64) != 56) {
        input.push_back(0);
    }

    // Append original length in bits
    uint64_t bitLength = originalLength * 8;
    for (size_t i = 0; i < 8; ++i) {
        input.push_back(static_cast<uint8_t>(bitLength >> (i * 8)));
    }

    uint32_t hash[4];
    md5Hash(input.data(), input.size(), hash);

    std::vector<uint8_t> hashBytes;
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            hashBytes.push_back(static_cast<uint8_t>(hash[i] >> (j * 8)));
        }
    }

    return byteArrayToHex(hashBytes);
}

// int main() {
//     std::string filename = "example.txt"; // Change to your file's name
//     std::string md5Value = calculateMD5(filename);

//     std::cout << "MD5 value for " << filename << ": " << md5Value << std::endl;

//     return 0;
// }

// #include <iostream>
// #include <fstream>
// #include <iomanip>
// #include <sstream>
// #include <string>
// #include <vector>
// #include "md5.h"

// // MD5 Round Constants
// const uint32_t MD5_K[] = {
//     0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476
// };

// // MD5 Shift Amounts
// const uint32_t MD5_S[] = {
//     7, 12, 17, 22,
//     5, 9, 14, 20,
//     4, 11, 16, 23,
//     6, 10, 15, 21
// };

// // MD5 Initialization Values (A, B, C, D)
// const uint32_t MD5_IV[] = {
//     0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476
// };

// // MD5 Padding
// const uint8_t MD5_PADDING[64] = {0x80};

// // Helper Functions
// inline uint32_t leftRotate(uint32_t value, uint32_t shift) {
//     return (value << shift) | (value >> (32 - shift));
// }

// void md5Hash(const uint8_t *input, size_t length, uint32_t hash[4]) {
//     uint32_t a = MD5_IV[0];
//     uint32_t b = MD5_IV[1];
//     uint32_t c = MD5_IV[2];
//     uint32_t d = MD5_IV[3];

//     // Main loop
//     for (size_t i = 0; i < length; i += 64) {
//         uint32_t words[16];
//         for (size_t j = 0; j < 16; ++j) {
//             words[j] = input[i + j * 4] | (input[i + j * 4 + 1] << 8) |
//                        (input[i + j * 4 + 2] << 16) | (input[i + j * 4 + 3] << 24);
//         }

//         uint32_t aa = a;
//         uint32_t bb = b;
//         uint32_t cc = c;
//         uint32_t dd = d;

//         for (size_t j = 0; j < 64; ++j) {
//             uint32_t f, g;

//             if (j < 16) {
//                 f = (b & c) | ((~b) & d);
//                 g = j;
//             } else if (j < 32) {
//                 f = (d & b) | ((~d) & c);
//                 g = (5 * j + 1) % 16;
//             } else if (j < 48) {
//                 f = b ^ c ^ d;
//                 g = (3 * j + 5) % 16;
//             } else {
//                 f = c ^ (b | (~d));
//                 g = (7 * j) % 16;
//             }

//             uint32_t temp = d;
//             d = c;
//             c = b;
//             b = b + leftRotate((a + f + MD5_K[j] + words[g]), MD5_S[j]);
//             a = temp;
//         }

//         a += aa;
//         b += bb;
//         c += cc;
//         d += dd;
//     }

//     hash[0] = a;
//     hash[1] = b;
//     hash[2] = c;
//     hash[3] = d;
// }

// std::string byteArrayToHex(const std::vector<uint8_t> &bytes) {
//     std::stringstream ss;
//     for (const uint8_t byte : bytes) {
//         ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
//     }
//     return ss.str();
// }

// std::string calculateMD5(const std::string &filename) {
//     std::ifstream file(filename, std::ios::binary);
//     if (!file) {
//         std::cerr << "Error opening file: " << filename << std::endl;
//         return "";
//     }
//     //TRACE("calculateMD5: File opened successfully");
//     //TRACE("good1 in calculateMD5: " + std::to_string(file.good()));
//     std::vector<uint8_t> input;
//     char buffer[64];
//     while (file.read(buffer, sizeof(buffer))) {
//         for (size_t i = 0; i < file.gcount(); ++i) {
//             input.push_back(buffer[i]);
//         }
//     }
//     //TRACE("good2 in calculateMD5: " + std::to_string(file.good()));
    
//     //TRACE("input vector size : " +to_string(input.size()));
//     // //TRACE("good: " + std::to_string(file.good()));
//     // //TRACE("bad: " + std::to_string(file.bad()));
//     // //TRACE("fail: " + std::to_string( file.fail()));
//     // //TRACE("eof: " + std::to_string( file.eof()));

//     for (int i = 0; i < 10; i++) {
//         std::cout << std::hex << (uint)(input[i]) << " ";
//     }
//     std::cout << std::endl;

//     for (int i = 41013; i < 41024; i++) {
//         std::cout << std::hex << (uint)(input[i]) << " ";
//     }

//     std::cout << std::endl;
//     // Add padding
//     size_t originalLength = input.size();
//     input.push_back(0x80);
//     while ((input.size() % 64) != 56) {
//         input.push_back(0);
//     }
//     // Append original length in bits
//     uint64_t bitLength = originalLength * 8;
//     for (size_t i = 0; i < 8; ++i) {
//         input.push_back(static_cast<uint8_t>(bitLength >> (i * 8)));
//     }

//     uint32_t hash[4];
//     md5Hash(input.data(), input.size(), hash);

//     std::vector<uint8_t> hashBytes;
//     for (size_t i = 0; i < 4; ++i) {
//         for (size_t j = 0; j < 4; ++j) {
//             hashBytes.push_back(static_cast<uint8_t>(hash[i] >> (j * 8)));
//         }
//     }
  
//     // file.close();
//     //TRACE("calculateMD5: File closed");
//     //TRACE("good3 in calculateMD5: " + std::to_string(file.good()));
//     return byteArrayToHex(hashBytes);
// }


// // std::string calculateMD5(char * buf, int len) {
// //     std::ifstream file(filename, std::ios::binary);
// //     if (!file) {
// //         std::cerr << "Error opening file: " << filename << std::endl;
// //         return "";
// //     }

// //     std::vector<uint8_t> input;
// //     char buffer[64];
// //     while (file.read(buffer, sizeof(buffer))) {
// //         for (size_t i = 0; i < file.gcount(); ++i) {
// //             input.push_back(buffer[i]);
// //         }
// //     }
// //     for (int i=0; i<len; i++) {
// //         input.push_back(buf[i]);
// //     }
    
// //     // Add padding
// //     size_t originalLength = input.size();
// //     input.push_back(0x80);
// //     while ((input.size() % 64) != 56) {
// //         input.push_back(0);
// //     }

// //     // Append original length in bits
// //     uint64_t bitLength = originalLength * 8;
// //     for (size_t i = 0; i < 8; ++i) {
// //         input.push_back(static_cast<uint8_t>(bitLength >> (i * 8)));
// //     }

// //     uint32_t hash[4];
// //     md5Hash(input.data(), input.size(), hash);

// //     std::vector<uint8_t> hashBytes;
// //     for (size_t i = 0; i < 4; ++i) {
// //         for (size_t j = 0; j < 4; ++j) {
// //             hashBytes.push_back(static_cast<uint8_t>(hash[i] >> (j * 8)));
// //         }
// //     }

// //     return byteArrayToHex(hashBytes);
// // }

// // int main() {
// //     std::string filename = "example.txt"; // Change to your file's name
// //     std::string md5Value = calculateMD5(filename);

// //     std::cout << "MD5 value for " << filename << ": " << md5Value << std::endl;

// //     return 0;
// // }
