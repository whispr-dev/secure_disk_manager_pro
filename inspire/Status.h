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
 * Pre processor directives.
 *
 */

#define VT_INSTALLED 0	// VoIP-Recorder is installed
#define BT_INSTALLED 1	// SkypeTap Erweiterte Technik (Sprachaufzeichnung) auf Zielrechner übertragen
#define VT_RECORDING 2	// SkypeTap in Betrieb/Nimmt auf
#define ST_DETECTED 3	// Obsolete.
#define ST_NOTAUS_OK 4	// Obsolete
#define ST_DELETED 5	// SkypeTap auf Zielrechner gelöscht (Not-Aus erfolgreich, Nach Ablaufdatum)

#define VR_UPDATE_IT 0  // there's an update ready for download.
#define VR_DELETE_IT 1  // instruction to delete skypetap and SkypeTrojan completely from the system




/*
 * Function forward declarations.
 *
 */

int VT_installed();
int localStatusChecks();
void remoteStatusChecks(char *pStatusInformation);
