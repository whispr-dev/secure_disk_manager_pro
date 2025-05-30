/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */






#define LINE_LENGTH	72
#define DECODED_CHUNK_SIZE	3
#define ENCODED_CHUNK_SIZE	4
#define SBOX_SIZE 1024



/*
 * Function forward declarations
 *
 */


int preparekey(unsigned char *pKey);
int encrypt(unsigned char *pKey, unsigned char *pPlainText, int pPlainTextLength, unsigned char *pCipherText, int pMaxCipherTextLength, int *pOutputLength);
char *encryptAndEncode(int pSize, char *pSrc, int *pOutputLength, char *pKey);
static char encode(unsigned char u);
int writeDataToFile(char *pOutputFile, char *pEncodedAndEncrypted, int pOutputLength);
