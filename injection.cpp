/*******************************************************************
 * SQL Injection Mitigation: Kevin Foniciello, Brian Aguilar, Paul Semadeni, Jared Aston, Chris Van Kammen
 *      This program will allow a user to select a mitigation type (no mitigation, weak , strong)
 *      and will run 5 different test cases with 5 different examples of usernames and passwords each for the following types of input:
 *      A valid input, a Tautology attack, a Union attack, an Additional Statement attack, and a Comment attack.
 *      The program will then display the original input vs the sanitized output and whether or not it passes the test case.
 *******************************************************************/

#include <iostream>
#include <string>
#include <regex>

using namespace std;
typedef string (*GenQueryFunction)(const string&, const string&);

struct Credentials;
struct TestSet;
string genQuery(const string&, const string&);
string genQueryWeak(const string&, const string&);
string genQueryStrong(const string&, const string&);
void runTestCases(char);
void runTests(TestSet&);
void testValid(TestSet&);
void testTautology(TestSet&);
void testUnion(TestSet&);
void testAddState(TestSet&);
void testComment(TestSet&);


int main() {
   char mitigationType = 'r';

   cout << "Options:\n"
        << "   r => Run tests on regular genQuery\n"
        << "   w => Run tests on weak    genQuery\n"
        << "   s => Run tests on strong  genQuery\n"
        << "   a => Run tests on on all  genQueries\n\n";
   bool wrongInputOption;
   do {
      cout << "> ";
      cin >> mitigationType;
      wrongInputOption = cin.fail() || (
                           mitigationType != 'r'
                           && mitigationType != 'w'
                           && mitigationType != 's'
                           && mitigationType != 'a'
                        );
      if (wrongInputOption) {
         cout << "Wrong option.\n";
      }
   } while(wrongInputOption);
   cout << endl;

   //Depending on type return proper test case
   runTestCases(mitigationType);

   return 0;
}

/*******************************************************************
 * genQuery():
 *      - Returns a SQL SELECT statement that relies on Username and Password.
 *******************************************************************/
string genQuery(const string & username, const string & password) {
    return "SELECT authenticate FROM passwordList WHERE name='" + username + "' and passwd='" + password + "';";
}

/*******************************************************************
 * genQueryWeak():
 *      - Strip the username and password of harmful and invalid characters contained in invalidChars[] array
 *******************************************************************/
string genQueryWeak(const string & username, const string & password) {
   string cleanUsername;
   string cleanPassword;

   //Go through each character in given username
   for (char c : username) {
      char invalidChars[] = {'<', '>', '&', ';', '"', '-', '=', '\'', ' ', '%', '#', '$', '(', ')', ','};
      bool isValid = true;

      //check if c exists in invalidChars
      for (char invalidChar : invalidChars) {
         //if it exists, add nothing to string and invalidate character
         if (c == invalidChar) {
            cleanUsername += "";
            isValid = false;
            break;
         }
      }

      //if character is still valid add it to string
      if (isValid) {
         cleanUsername += c;
      }
   }

   //Go through each character in given password
   for (char c : password) {
      char invalidChars[] = {'<', '>', '&', ';', '"', '-', '=', '\'', ' ', '(', ')', ','};
      bool isValid = true;

      //check if c exists in invalidChars
      for (char invalidChar : invalidChars) {
         //if it exists, add nothing to string and invalidate character
         if (c == invalidChar) {
            cleanPassword += "";
            isValid = false;
            break;
         }
      }

      //if character is still valid add it to string     
      if (isValid) {
         cleanPassword += c;
      }
   }

   return "SELECT authenticate FROM passwordList WHERE name='" + cleanUsername + "' and passwd='" + cleanPassword + "';";
}

/*******************************************************************
 * genQueryStrong():
 *       - Username: Allow only letters, numbers, ., and _
 *       - Password: Allow only letters, numbers, and special characters (!@#$%^&:*._)
 *******************************************************************/
string genQueryStrong(const string & username, const string & password) {
   string cleanUsername;
   string cleanPassword;
   for (char c : username) {
      if (isalnum(c) || c == '.' || c == '_')
         cleanUsername += c;
   }
   for (char c : password) {
      string s(1, c);
      if (isalnum(c) || regex_match(s, regex("[\\w!@#$%^&:*_.]")))
         cleanPassword += c;
   }

   return "SELECT authenticate FROM passwordList WHERE name='" + cleanUsername + "' and passwd='" + cleanPassword + "';";
}


/*******************************************************************
 * TESTS
 *******************************************************************/

struct Credentials {
   Credentials(string username, string password) : username(std::move(username)), password(std::move(password)) {}

   string username;
   string password;
};
struct TestCase {
   Credentials input;
   string expectedOutput;
   string errorMessage;
};
struct TestSet {
   string title;
   string name;
   GenQueryFunction functionTested = nullptr;
   int testsNumber = 0;
   int testsPassedCount = 0;
   TestCase *testCases = nullptr;
};


void runTestCases(char mitigationType) {
   TestSet testSets[3];
   string setGroupTitle;
   switch (mitigationType) {
      /// Regular
      case 'r': {
         testSets[0].title = "genQuery()";
         testSets[0].functionTested = genQuery;
         break;
      }
      /// Weak
      case 'w': {
         testSets[0].title = "genQueryWeak()";
         testSets[0].functionTested = genQueryWeak;
         break;
      }
      /// Strong
      case 's': {
         testSets[0].title = "genQueryStrong()";
         testSets[0].functionTested = genQueryStrong;
         break;
      }
      /// All
      case 'a': {
         setGroupTitle = "RUNNING TESTS ON ALL GEN_QUERIES...";
         testSets[0].title = "genQuery()";
         testSets[0].functionTested = genQuery;
         testSets[1].title = "genQueryWeak()";
         testSets[1].functionTested = genQueryWeak;
         testSets[2].title = "genQueryStrong()";
         testSets[2].functionTested = genQueryStrong;
         break;
      }

      default:
         throw "Uh oh, wrong option...";
   }

   /// Run all tests
   if (mitigationType == 'a') {
      cout << setGroupTitle << endl;

      int totalTestsPassedCount = 0;
      for (TestSet & testSet : testSets) {
         int testsPassedCount = 0;
         cout << "   Running set \"" << testSet.title << "\"...\n";
         testValid(testSet);
         testTautology(testSet);
         testUnion(testSet);
         testAddState(testSet);
         testComment(testSet);
         totalTestsPassedCount += testSet.testsPassedCount;
         testsPassedCount += testSet.testsPassedCount;
         cout << "   **************************************************************************\n"
              << "   *  " << testsPassedCount << "/25 tests passed for \"" << testSet.title << "\"\n"
              << "   **************************************************************************\n\n";
      }
      cout << "==========================================================================\n"
           << "|| TOTAL: " << totalTestsPassedCount << "/" << 3*25 << " tests passed\n"
           << "==========================================================================\n\n";
   }
   /// Run the specified test
   else {
      cout << "Running set \"" << testSets[0].title << "\"...\n";
      testValid(testSets[0]);
      testTautology(testSets[0]);
      testUnion(testSets[0]);
      testAddState(testSets[0]);
      testComment(testSets[0]);

      cout << "**************************************************************************\n"
           << "*  " << testSets[0].testsPassedCount << "/25 tests passed for \"" << testSets[0].title << "\"\n"
           << "**************************************************************************\n\n";
   }
}

void testValid(TestSet& testSet) {
   /// setup
   testSet.name = "Valid input";
   testSet.testsNumber = 5;
   string errorMessage = "Query not properly build with VALID input";
   testSet.testCases = new TestCase[5] {
      // { username, password, expected output }
           { Credentials("Jane_Doe", "7593156"), "SELECT authenticate FROM passwordList WHERE name='Jane_Doe' and passwd='7593156';", errorMessage }, // Jared
           { Credentials("w4shingt0nx4vier", "509@pp1e"), "SELECT authenticate FROM passwordList WHERE name='w4shingt0nx4vier' and passwd='509@pp1e';", errorMessage }, // Paul
           { Credentials("Brian_064", "123secret123"), "SELECT authenticate FROM passwordList WHERE name='Brian_064' and passwd='123secret123';", errorMessage }, //Brian
           { Credentials("Kevin123456", "wh@t$MyPassw0rd"), "SELECT authenticate FROM passwordList WHERE name='Kevin123456' and passwd='wh@t$MyPassw0rd';", errorMessage }, // Kevin
           { Credentials("simpleWords", "********"), "SELECT authenticate FROM passwordList WHERE name='simpleWords' and passwd='********';", errorMessage } //chris1
   };

   /// test & validate
   runTests(testSet);

   //delete testSet.testCases;
   testSet.testCases = nullptr;
}


void testTautology(TestSet& testSet) {
   /// setup
   testSet.name = "Tautology exploitation";
   testSet.testsNumber = 5;
   string errorMessage = "Query failed due to a BOOLEAN attack";
   testSet.testCases = new TestCase[5] {
           // { username, password, expected output }
           { Credentials("Jane_Doe", "fake_password' OR 'x' = 'x"), "SELECT authenticate FROM passwordList WHERE name='Jane_Doe' and passwd='fake_passwordORxx';", errorMessage }, // Jared
           { Credentials("w4shingt0nx4vier", "this_should_work' OR 'a' = 'a"), "SELECT authenticate FROM passwordList WHERE name='w4shingt0nx4vier' and passwd='this_should_workORaa';", errorMessage }, // Paul
           { Credentials("Brian_064", "lol' OR '1' = '1"), "SELECT authenticate FROM passwordList WHERE name='Brian_064' and passwd='lolOR11';", errorMessage }, //Brian
           { Credentials("alexanderHamilton#1776", "c' OR 'c'='c"), "SELECT authenticate FROM passwordList WHERE name='alexanderHamilton1776' and passwd='cORcc';", errorMessage }, // Kevin
           { Credentials("homeStarLegend%$#", "takingout OR date"), "SELECT authenticate FROM passwordList WHERE name='homeStarLegend' and passwd='takingoutORdate';", errorMessage } //chris2
   };

   /// test & validate
   runTests(testSet);

   //delete testSet.testCases; 
   testSet.testCases = nullptr;
}


void testUnion(TestSet& testSet) {
   /// setup
   testSet.name = "Union exploitation";
   testSet.testsNumber = 5;
   string errorMessage = "Query was altered to execute a UNION";
   testSet.testCases = new TestCase[5] {
           // { username, password, expected output }
           { Credentials("Jane_Doe", "testing' UNION INSERT INTO  passwordList(name, passwd) VALUES('Jane Doe', 'my_password')"), "SELECT authenticate FROM passwordList WHERE name='Jane_Doe' and passwd='testingUNIONINSERTINTOpasswordListnamepasswdVALUESJaneDoemy_password';", errorMessage }, //Jared
           { Credentials("w4shingt0nx4vier", "this_should_work' UNION INSERT INTO passwordList(name, passwd) VALUES ('w4shingt0nx4vier', '509@pp1e')"), "SELECT authenticate FROM passwordList WHERE name='w4shingt0nx4vier' and passwd='this_should_workUNIONINSERTINTOpasswordListnamepasswdVALUESw4shingt0nx4vier509@pp1e';", errorMessage }, // Paul
           { Credentials("Brian_064", "myPassword' UNION SELECT * FROM passwordList;"), "SELECT authenticate FROM passwordList WHERE name='Brian_064' and passwd='myPasswordUNIONSELECT*FROMpasswordList';", errorMessage }, //Brian
           { Credentials("ronWeasly", "alohamora' UNION SELECT * FROM passwordList WHERE 'x'='x"), "SELECT authenticate FROM passwordList WHERE name='ronWeasly' and passwd='alohamoraUNIONSELECT*FROMpasswordListWHERExx';", errorMessage }, // Kevin
           { Credentials("wizardgandolf", "you are being hacked UNION SELECT "), "SELECT authenticate FROM passwordList WHERE name='wizardgandolf' and passwd='youarebeinghackedUNIONSELECT';", errorMessage } // chris3
   };
   /// test & validate
   runTests(testSet);

   //delete testSet.testCases;
   testSet.testCases = nullptr;
}


void testAddState(TestSet& testSet) {
   /// setup
   testSet.name = "Add State exploitation";
   testSet.testsNumber = 5;
   string errorMessage = "A state was added to the query";
   testSet.testCases = new TestCase[5] {
           // { username, password, expected output }
           { Credentials("Jane_Doe", "0; SELECT password FROM passwordList"), "SELECT authenticate FROM passwordList WHERE name='Jane_Doe' and passwd='0SELECTpasswordFROMpasswordList';", errorMessage }, // Jared
           { Credentials("w4shingt0nx4vier", "this_should_work'; SELECT * FROM passwordList WHERE password LIKE '%5%"), "SELECT authenticate FROM passwordList WHERE name='w4shingt0nx4vier' and passwd='this_should_workSELECT*FROMpasswordListWHEREpasswordLIKE%5%';", errorMessage }, // Paul
           { Credentials("Brian_064", "mypassWord'; delete FROM table;"), "SELECT authenticate FROM passwordList WHERE name='Brian_064' and passwd='mypassWorddeleteFROMtable';", errorMessage }, //Brian
           { Credentials("brian_is_cool", "000'; SELECT * FROM passwordList where 'c'='c"), "SELECT authenticate FROM passwordList WHERE name='brian_is_cool' and passwd='000SELECT*FROMpasswordListwherecc';", errorMessage }, // Kevin
           { Credentials("poleroidMan", "; SELECT * FROM passwordList LEFT JOIN myOwnTable on passwordList.passwords=myOwnTable.passwords;"), "SELECT authenticate FROM passwordList WHERE name='poleroidMan' and passwd='SELECT*FROMpasswordListLEFTJOINmyOwnTableonpasswordList.passwordsmyOwnTable.passwords';", errorMessage } //chris4
   };

   /// test & validate
   runTests(testSet);

   //delete testSet.testCases;
   testSet.testCases = nullptr;
}


void testComment(TestSet& testSet) {
   /// setup
   testSet.name = "Comment exploitation";
   testSet.testsNumber = 5;
   string errorMessage = "Comments altered how the query should have run";
   testSet.testCases = new TestCase[5] {
           // { username, password, expected output }
           { Credentials("Jane_Doe'--", "false_password"), "SELECT authenticate FROM passwordList WHERE name='Jane_Doe' and passwd='false_password';", errorMessage }, // Jared
           { Credentials("w4shingt0nx4vier'; --", "509@pp1e'; --"), "SELECT authenticate FROM passwordList WHERE name='w4shingt0nx4vier' and passwd='509@pp1e';", errorMessage }, // Paul
           { Credentials("Brian_064' --", "anyPassWord"), "SELECT authenticate FROM passwordList WHERE name='Brian_064' and passwd='anyPassWord';", errorMessage }, //Brian
           { Credentials("chris_is_awesome", "5stars'; -- this_is_a_comment:_"), "SELECT authenticate FROM passwordList WHERE name='chris_is_awesome' and passwd='5starsthis_is_a_comment:_';", errorMessage }, // Kevin
           { Credentials("poleroidMan; --", "password"), "SELECT authenticate FROM passwordList WHERE name='poleroidMan' and passwd='password';", errorMessage }//chris5
   };

   /// test & validate
   runTests(testSet);

   //delete testSet.testCases;
   testSet.testCases = nullptr;
}


 void runTests(TestSet & testSet) {
    cout << "\t" << testSet.name << ":\n";

    int testsPassedCount = 0;
    for (int i = 0; i < testSet.testsNumber; i++) {
       const string username = testSet.testCases[i].input.username;
       const string password = testSet.testCases[i].input.password;
       cout << "\t   Input (username/password):\n"
            << "\t      " << "Username: " << username << endl
            << "\t      " << "Password: "<< password << endl;

       /// Test
       const string actualOutput = testSet.functionTested(username, password);

       /// Verify
       if (actualOutput == testSet.testCases[i].expectedOutput) {
         testSet.testsPassedCount++;
         testsPassedCount++;
         cout << "\t      PASSED: Inputs are clean/sanitized\n"
              << "\t         Actual   output: " << actualOutput << endl
              << "\t         Expected output: " << testSet.testCases[i].expectedOutput << endl;
       } else {
         cout << "\t      FAILED: " << testSet.testCases[i].errorMessage << endl
              << "\t         Actual   output: " << actualOutput << endl
              << "\t         Expected output: " << testSet.testCases[i].expectedOutput << endl << endl << endl;
       }
    }

    /// Total
    cout << "\t==========================================================================\n"
         << "\t|| " << testsPassedCount << "/" << testSet.testsNumber << " tests passed for " << testSet.name << endl
         << "\t==========================================================================\n\n";
 }
