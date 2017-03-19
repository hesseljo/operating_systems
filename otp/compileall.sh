#!/bin/bash
rm -r otp_dec
rm -r otp_dec_d
rm -r otp_enc
rm -r otp_enc_d
rm -r keygen

gcc otp_dec.c -o otp_dec -std=c99
gcc otp_dec_d.c -o otp_dec_d -std=c99
gcc otp_enc.c -o otp_enc -std=c99
gcc otp_enc_d.c -o otp_enc_d -std=c99
gcc keygen.c -o keygen -std=c99