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





/*
 * Pre processor directives
 *
 */

#define SBOX_SIZE 1024




/*
 * Function forward declarations
 *
 */

static unsigned char decodeFromB64(char pCharacter);
static char encodeToB64(unsigned char u);
int preparekey(unsigned char *pKey);
int encrypt(unsigned char *pKey, unsigned char *pPlainText, int pPlaintextLength, unsigned char *pCipherText, int *pOutputLength);
int decrypt(unsigned char *pKey, unsigned char *pPlainText, int pPlaintextLength, unsigned char *pCipherText, int *pOutputLength);
static int charIsB64(char pCharacter);
