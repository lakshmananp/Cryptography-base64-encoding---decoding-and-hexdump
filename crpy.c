
#include <stdio.h>
#include <string.h>

// Encode Base64 values in input received to ASCII
void dec_base64(FILE *in) {
        char buf[1];
        unsigned char curr[4];
        unsigned char out[3];
        int bytes_read = 0;
        bool done = false;
        
        // Iterate through the entire input by bytes
        while(fread(buf, 1, 1, in) > 0) {
        
                // Check to see if byte read is valid input, align it with Base64 alphabet if it is
                if(buf[0] == '\n') {
                        continue;
                } else if(buf[0] >= 'A' && buf[0] <= 'Z') {
                        curr[bytes_read] = buf[0] - 'A';
                } else if(buf[0] >= 'a' && buf[0] <= 'z') {
                        curr[bytes_read] = buf[0] - 'a' + 0x1a;
                } else if(buf[0] >= '0' && buf[0] <= '9') {
                        curr[bytes_read] = buf[0] - '0' + 0x34;
                } else if(buf[0] == '+') {
                        curr[bytes_read] = 0x3e;
                } else if(buf[0] == '/') {
                        curr[bytes_read] = 0x3f;
                } else if(buf[0] == '=') {
                        // Reached end of input stream, indicate ending
                        curr[bytes_read] = 0;
                        done = true;
                } else {
                        fprintf(stderr, "ERROR: input stream contained invalid Base64 characters (%c).\n", buf[0]);
                        return;
                }
                if(!done)
                        bytes_read++;
                
                // Check if enough bytes have been read to convert this block back to ASCII
                if(bytes_read == 4 || done) {
                        // Convert the 4 Base64 characters to ASCII bytes
                        out[0] = (curr[0] << 2) | ((curr[1] & 0x30) >> 4);
                        if(bytes_read >= 3)
                                out[1] = ((curr[1] & 0x0f) << 4) | ((curr[2] & 0x3c) >> 2);
                        if(bytes_read == 4)
                                out[2] = ((curr[2] & 0x03) << 6) | (curr[3] & 0x3f);
        
                        for(int i=0; i<(bytes_read-1); i++) {
                                printf("%c", out[i]);
                        }
                        
                        if(done) {
                                // Hit end, done
                                break;
                        } else {
                                curr[0] = curr[1] = curr[2] = curr[3] = 0;
                                bytes_read = 0;
                        }
                }
                
        }
        
       
}


char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";



void enc_base64(FILE *in) {
        char buf[3];
        unsigned char curr[3];
        unsigned char out[4];
        int bytes_read;
        int column_count = 0;
        
        // Initial reading of input
        bytes_read = fread(buf, 1, 3, in);
        
        // Iterate through the entire input by bytes
        while(bytes_read > 0) {
                for(int i=0; i<3; i++) {
                        curr[i] = buf[i];
                }
                
                // Has stored three characters, convert to base64
                out[0] = alpha[ curr[0] >> 2 ];
                out[1] = alpha[ ((curr[0] & 0x03) << 4) | ((curr[1] & 0xf0) >> 4) ];
                if(bytes_read > 1) {
                        out[2] = alpha[ ((curr[1] & 0x0f) << 2) | ((curr[2] & 0xc0) >> 6) ];
                } else {
                        out[2] = '=';
                }
                if(bytes_read > 2) {
                        out[3] = alpha[ curr[2] & 0x3f ];
                } else {
                        out[3] = '=';
                }
                
                // Print the output to the screen as we go, newline after 64 chars
                for(int i=0; i<4; i++) {
                        printf("%c", out[i]);
                        column_count++;
                        if(column_count >= 64) {
                                printf("\n");
                                column_count = 0;
                        }
                }
                
                // Clear existing buffer and read next block in
                memset(buf, 0, sizeof(char)*3);
                if(bytes_read > 0)
                        bytes_read = fread(buf, 1, 3, in);
        }
        
        // Append newline to output, in case one is needed
        if(column_count != 0) {
                printf("\n");
        }
        
       
}

// Prints plaintext output at end of row
void printLineOut(char lineOut[]) {
        // Print row overview in ASCII here
        for(unsigned int i=0; i<16; i++) {
                printf("%c", lineOut[i]);
        }
        printf("\n");
}

// Complete a hexdump of ASCII characters received
void hexdump(FILE *in) {
        char buf[1];
        char lineOut[16];
        unsigned char charRead;
        int numBytes = 0;
        int bytesInRow = 0;
        
        // Iterate through the entire input by bytes
        while(fread(buf, 1, 1, in) > 0) {
                charRead = buf[0];                
                
                if(bytesInRow == 0) {
                        printf("%06x: ", numBytes);
                }
                
                // Processing of the byte read
                printf("%02x ", charRead);
                if(charRead >= 32 && charRead <= 126) {
                        lineOut[bytesInRow] = charRead;
                } else if(charRead <= 31 || charRead == 127) {
                        lineOut[bytesInRow] = '.';
                } else {
                        lineOut[bytesInRow] = '~';
                }
                numBytes++;
                bytesInRow++;
                if(bytesInRow%8 == 0)
                        printf(" ");
                
                // End of the row, print final row output
                if(bytesInRow == 16) {
                        bytesInRow = 0;
                        printLineOut(lineOut);
                }
        }
        
        // Account for excess space at the end of the input
        if(bytesInRow > 0) {
                for(unsigned int i = bytesInRow; i<sizeof(lineOut); i++) {
                        // Set unused byte values to '--'
                        printf("-- ");
                        lineOut[bytesInRow] = ' ';
                        numBytes++;
                        bytesInRow++;
                        if(bytesInRow%8 == 0)
                                printf(" ");
                                
                        // End of the row
                        if(bytesInRow == 16) {
                                bytesInRow = 0;
                                printLineOut(lineOut);
                        }
                }
        }
        
      
}

int main(int argc, char *argv[]) {

        // Look for correct command line arguments
        if(argc <= 1 || argc > 3) {
                fprintf(stderr, "ERROR: insufficient parameters. Should be: 'hw1 (hexdump/enc-base64/dec-base64) [file]'\n");
                return 0;
        } else {
                if(strcmp(argv[1], "hexdump") == 0) {
                        // Hexdump prog should be run
                        if(argc == 3) {
                                // Take input from file
                                FILE *fin = NULL;
                                fin = fopen(argv[2], "r");
                                if(fin == NULL) {
                                        fprintf(stderr, "ERROR: invalid file '%s' for reading.\n", argv[2]);
                                        return 0;
                                }
                                hexdump(fin);
                                fclose(fin);
                        } else {
                                // Take input from cmd line
                                hexdump(stdin);
                        }
                }
                else if(strcmp(argv[1], "enc-base64") == 0) {
                        // Encode to Base64 prog should run
                        if(argc == 3) {
                                // Take input from file
                                FILE *fin = NULL;
                                fin = fopen(argv[2], "r");
                                if(fin == NULL) {
                                        fprintf(stderr, "ERROR: invalid file '%s' for reading.\n", argv[2]);
                                        return 0;
                                }
                                enc_base64(fin);
                                fclose(fin);
                        } else {
                                // Take input from cmd line
                                enc_base64(stdin);
                        }
                }
                else if(strcmp(argv[1], "dec-base64") == 0) {
                        // Decode from Base64 prog should run
                        if(argc == 3) {
                                // Take input from file
                                FILE *fin = NULL;
                                fin = fopen(argv[2], "r");
                                if(fin == NULL) {
                                        fprintf(stderr, "ERROR: invalid file '%s' for reading.\n", argv[2]);
                                        return 0;
                                }
                                dec_base64(fin);
                                fclose(fin);
                        } else {
                                // Take input from cmd line
                                dec_base64(stdin);
                        }
                }
                else {
                        fprintf(stderr, "ERROR: invalid parameters. Should be: 'hw1 (hexdump/enc-base64/dec-base64) [file]'\n");
                        return 0;
                }
        }
        
        return 0;
}
