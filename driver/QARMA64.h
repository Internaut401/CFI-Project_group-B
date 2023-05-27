#include "./QARMA64.c"

text_t qarma64_enc(text_t plaintext, tweak_t tweak, qkey_t w0, qkey_t k0, int rounds);
text_t qarma64_dec(text_t plaintext, tweak_t tweak, qkey_t w0, qkey_t k0, int rounds);
