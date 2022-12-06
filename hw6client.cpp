#include <iostream>
#include "ecs36b_Exception.h"
#include <cstring>

#include <string>
#include <string.h>
#include <iostream>
#include <fstream>

// JSON RPC part
#include "hw6client.h"
#include <jsonrpccpp/client/connectors/httpclient.h>

using namespace jsonrpc;
using namespace std;

#define WORD_LENGTH 5

char alphabet[] = "abcdefghijklmnopqrstuvwxyz"; //alphabet[0] = a, [1] = b, [25]
char includedAlphabet[] = ".........................."; // letters that are included in the correct word (# or o)
char prevGuess[] = "-----"; // keeps track of what our guess was
char correctGuess[] = "-----"; // keeps track of the correct position letters
char nextGuess[] = "-----"; // new/next word to guess

char resultKey[] = "_____"; // result recieved from server
char correctKey[] = "#####" // correct word result
//  _ Invalid Letter
//  # Correct Letter
//  o Wrong Position

void findNextGuess() {

  // FILE HANDLING
  FILE * fpSol;
  char grabbedWord[WORD_LENGTH + 1] = {0};
  fpSol = fopen("lists/SOLUTION.TXT", "r");

  if (fpSol == NULL) {
		printf("Error opening wordle lists");
	}

  // CHECKS FOR NEXT VALID GUESS
  bool foundNextGuess = false;

  while ( (fgets(grabbedWord, WORD_LENGTH + 1, fpSol) != NULL) && (!foundNextGuess) ) {
    if (strlen(grabbedWord) == WORD_LENGTH) {
      
      bool validGuess = true;

      // Goes through each letter of grabbedWord (from solution list) and checks for Correct Position and Invalid Letters
      for (int i = 0; i < WORD_LENGTH; i++){
        
        // If we have a correct position letter at index i, check if grabbedWord letter at index i matches as well
        if ((correctGuess[i] != '-') && (correctGuess[i] != grabbedWord[i])) {
          validGuess = false; // mark word as not valid
        }

        // Check to see if letter is exluded from alphabet
        bool excludedLetter = true;
        for (int j = 0; j < 26; j++) {
          if (grabbedWord[i] == alphabet[j]) {
            excludedLetter = false;
          }
        }
        if (excludedLetter){
          validGuess = false;
        }
      }

      // Check to see if the word includes each of the includedAlphabet letters at least once
        for (int i = 0; i < 26; i++) {
          if ((includedAlphabet[i] != '.')) {
            bool isIncluded = false;
            for (int j = 0; j < WORD_LENGTH; j++) {
              if(grabbedWord[j] == includedAlphabet[i])
                isIncluded = true;
            }
            if(!isIncluded){
              validGuess = false;
            }
          }

        }

      if (validGuess){
        for (int i = 0; i < WORD_LENGTH; i++){
          nextGuess[i] = grabbedWord[i];
        } 

        foundNextGuess = true;
      }
    }

  }

  fclose(fpSol);
  
}

void updateKeys() {

   // Update alphabet, validGuess, and included
    for (int i = 0; i < WORD_LENGTH; i++){
      if ( resultKey[i] == '#') { // Correct Letter
        
        correctGuess[i] = prevGuess[i];

        // Puts the included letter into includedAlphabet
        for (int j = 0; j < 26; j++) { 
          if (alphabet[j] == prevGuess[i]){
            includedAlphabet[j] = prevGuess[i];
          }
        }

      } else if (resultKey[i] == 'o') { // Incorrect Position Letter
        
        // Puts the correct position letter into includedAlphabet
        for (int j = 0; j < 26; j++) { 
          if (alphabet[j] == prevGuess[i]){
            includedAlphabet[j] = prevGuess[i];
          }
        }

      } else if (resultKey[i] == '_') { // Invalid Letter
        
        // Gets rid of invalid letters in alphabet
        for (int j = 0; j < 26; j++) { 
          if (alphabet[j] == prevGuess[i]){
            alphabet[j] = '_';
          }
        }

      }
    }

}

int
main()
{

  HttpClient httpclient("https://2ada-73-66-168-157.ngrok.io");
  hw6Client myClient(httpclient, JSONRPC_CLIENT_V2);
  Json::Value myv;
  Json::Value jv_list;
  
  jv_list[0] = "919227725";
  jv_list[1] = "918470625";
  jv_list[2] = "917137244";
  jv_list[3] = "919388431";
  
  // sets team name
  try {
    myv = myClient.set_name("set_name", jv_list, "Team please give us an A");
  } catch (JsonRpcException &e) {
    cerr << e.what() << endl;
  }
  std::cout << myv.toStyledString() << std::endl;

  // start new game
  try {
    myv = myClient.obtain("obtain", "Wordle", "00000000");
  } catch (JsonRpcException &e) {
    cerr << e.what() << endl;
  }
  std::cout << "obtain" << myv.toStyledString() << std::endl;

  // start guessing
  if ((myv["game_id"].isNull() == false) &&
      (myv["game_id"].isString() == true))
    {
      try {
	// First Guess
	myv = myClient.guess("guess", "Wordle", myv["game_id"].asString(),
			     "afire");
      } catch (JsonRpcException &e) {
	cerr << e.what() << endl;
      }

      try {
	// Second Guess
	myv = myClient.guess("guess", "Wordle", myv["game_id"].asString(),
			     "wound");
      } catch (JsonRpcException &e) {
	cerr << e.what() << endl;
      }

      try {
	// Third Guess
	myv = myClient.guess("guess", "Wordle", myv["game_id"].asString(),
			     "batch");
      } catch (JsonRpcException &e) {
	cerr << e.what() << endl;
      }

      std::cout << myv.toStyledString() << std::endl;
    }
  
    string strPrevGuess = "batch";
    strcpy(prevGuess,strPrevGuess.c_str());
    string strResultKey = myv["check_word"].asString();
    strcpy(resultKey,strResultKey.c_str());
  
  bool gameSubmitted = false;

  while (!gameSubmitted){

    if ( strcmp(resultKey,correctKey) == 0 ) {


      cout << "Guess is correct. Submitting Game...";


      try {
      // Submit Completed Game
      myv = myClient.submit("submit", myv["game_id"].asString(), "Team please give us an A");
    } catch (JsonRpcException &e) {
      cerr << e.what() << endl;
    }
    std::cout << myv.toStyledString() << std::endl;

    gameSubmitted = true;

    } else {

      // update alphabet, includeAlphabet, and valid Guess,
      updateKeys();

      // Gets a new guess to send to server, and puts to nextGuess
      findNextGuess();

      // Convert our nextGuess[] into a string ready for input
      string strNextGuess = nextGuess;

      // Iterated Guesses
      if ((myv["game_id"].isNull() == false) &&
          (myv["game_id"].isString() == true))
        {
          try {
      // First Guess
      myv = myClient.guess("guess", "Wordle", myv["game_id"].asString(),
              strNextGuess);
          } catch (JsonRpcException &e) {
      cerr << e.what() << endl;
          }

          std::cout << myv.toStyledString() << std::endl;
        }

      string strResultKey = myv["check_word"].asString();
      strcpy(resultKey,strResultKey.c_str());

    }

  }

  return 0;
}


