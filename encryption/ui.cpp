/********************************************************************
* Program:
*    Lab 13: Create a cipher
* Author:
*    Br. Helfrich
* Summary:
*    This program will be able to encrypt and decrypt text using a
*    variety of ciphers.
*********************************************************************/
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "cipher.h"             // Base Cipher Class
#include "example.h"            // Bro Helfrich example Cipher
#include "caesarisQuadratum.h"  // Kevin Foniciello's cypher
#include "ZigZagRoute.h"        // Brian Aguilar's cypher
#include "Baconian.h"           // Paul Semadeni's cypher
#include "fourSquare.h"         // Jared Aston's cypher
#include "cipher05.h"           // Include the other 3 header files
#include "cipher06.h"
#include "Substitution.h"

//#define DEMO // Demo mode
#ifdef DEMO
   #define SLEEP "sleep 1.5"
   #define MAX_LINES 25
#endif

using namespace std;

/********************************************************************
 * UI CLASS
 ********************************************************************/
class UI
{
private:
   vector<Cipher*> cipher;
   string plaintext;
   string password;

public:
   /************************************************************
    * DEFAULT CONSTRUCTOR
    * sets string to null and loads the cipher vector
    ************************************************************/
   UI()
   {
      plaintext = "";
      password = "";

      // load the ciphers into the cipher vector
      cipher.push_back(new Example);  // add the example Cipher
      cipher.push_back(new CaesarisQuadratum);
      cipher.push_back(new ZigZagRoute);
      cipher.push_back(new Baconian);
      cipher.push_back(new FourSquare);
      cipher.push_back(new Cipher05); // add other 3 Ciphers
      cipher.push_back(new Cipher06);
      cipher.push_back(new Substitution);
   }

   /************************************************************
    * MENU
    * displays the available ciphers and returns the option
    ************************************************************/
   int menu()
   {
      int nOptions = cipher.size();

      // display the options
      cout << "What cipher do you want to test? (-1 to quit)\n";
      for (int i = 0; i != nOptions; i++)
      {
         string nCipher = cipher[i]->getCipherName();
         if (nCipher.compare("cipher name") != 0)
            cout << setw(4) << i << " .... " 
                 << cipher[i]->getCipherName() << endl;
      }

      // get the users option and return it
      return getOption();
   }

   /************************************************************
    * GET OPTION
    * prompts for the option and returns the correct index
    ************************************************************/
   int getOption()
   {
      int nOptions = cipher.size();
      int option = -1;

      while (option < 0 || option > nOptions)
      {
         cout << "> ";
         cin >> option;
         
         // check for a value that is not an int
         if (cin.fail())
         {
            cout << "ERROR - non-digit entered\n";
            cin.clear();
            cin.ignore(256, '\n');
         }
         else if (option == -1)
            break;
         else if (option < 0 || option > nOptions)
            cout << "ERROR - value is outside the accepted range\n";
         else
            cin.ignore(); // clear the buffer of any leftovers...
      }

      return option; 
   }

   /************************************************************
    * GET TEXT
    * get the plaintext and password from the user
    ************************************************************/
   void getText()
   {
      // get the text
      cout << "Please enter the text: ";
      getline(cin, plaintext);
      if (plaintext.compare("") == 0)
         plaintext = "I am just \"plain\" text.";

      // get the password
      cout << "Please enter the password: ";
      getline(cin, password);
      if (password.compare("") == 0)
      {
         password = "Passw0rd!";
         cout << "Default password: " << password << endl;
      }
   }

   /************************************************************
    * GET REPORT
    * generates the report for the selected cipher
    ************************************************************/
   void getReport(const int & index)
   {
      string encrypted = cipher[index]->encrypt(plaintext, password);
      string decrypted = cipher[index]->decrypt(encrypted, password);
      cout << "==================================="
           << "===================================\n";
      cout << "Cipher Name:         " 
           << cipher[index]->getCipherName() << endl;
      cout << "Pseudocode Author:   " 
           << cipher[index]->getPseudoAuth() << endl;
      cout << "Code Encrypt Author: " 
           << cipher[index]->getEncryptAuth() << endl;
      cout << "Code Decrypt Author: " 
           << cipher[index]->getDecryptAuth() << endl;
      cout << "==================================="
           << "===================================\n";
#ifdef DEMO
      system(SLEEP);
#endif
      cout << "Citation:\n"
           << cipher[index]->getCipherCitation() << endl;
      cout << "==================================="
           << "===================================\n";
#ifdef DEMO
      system(SLEEP);
#endif
      cout << "Plain text:    " << plaintext << endl;
      cout << "Cipher text:   " << encrypted << endl;
      cout << "Decipher text: " << decrypted << endl;
      cout << "==================================="
           << "===================================\n";
#ifdef DEMO
      system(SLEEP);
#endif
      cout << "Pseudocode:\n";

#ifdef DEMO
      int i = 0;
      bool slept = true;
      cout << setw(4) << left << i+1 ;
      for (const char c : cipher[index]->getPseudocode()) {
         cout << c ;

         if (c == '\n') {
            i++;
            cout << setw(4) << left << i+1 ;
            slept = false;
         }

         if (!(i % MAX_LINES) && !slept) {
            system(SLEEP);
            slept = true;
         }
      }
#else
      cout << cipher[index]->getPseudocode() << endl;
#endif
   }
};

/********************************************************************
 * MAIN
 * drives the UI class
 ********************************************************************/
int main()
{
   UI interface;
   int index;

   while (true) {
      index = interface.menu(); // show the menu and get cipher index

      if (index == -1) // if index == -1 then quit
         return 0;

      interface.getText(); // get the plaintext and password
      interface.getReport(index); // generate the report

#ifdef DEMO
      system(SLEEP);
      system(SLEEP);
      system("cls");
      system("clear");
#endif
   }
}