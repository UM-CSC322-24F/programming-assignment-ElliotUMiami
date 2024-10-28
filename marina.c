#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Macro values
#define MAX_BOATS 120
#define NAME_LENGTH 128
#define LICENSE_LENGTH 7

//Enum for the place variable
typedef enum {
  slip,
  land,
  trailor,
  storage,
  no_place
} PlaceType;

//Union for the extra variable
typedef union {
  int slip_num;
  char bay_letter;
  char license_tag[LICENSE_LENGTH];
  int storage_num;
} ExtraInfoType;

//Struct to record information about a boat
typedef struct {
  char name[NAME_LENGTH];
  int length;
  PlaceType place;
  ExtraInfoType extra;
  float amount_owed;
} Boat;

//Array of pointers to boats
Boat * marina[MAX_BOATS];

//Converts a string to a place
PlaceType stringToPlaceType(char * place_string) {
    if (!strcasecmp(place_string,"slip")) {
      return(slip);	
    }
    if (!strcasecmp(place_string,"land")) {
        return(land);
    }
    if (!strcasecmp(place_string,"trailor")) {
        return(trailor);
    }
    if (!strcasecmp(place_string,"storage")) {
        return(storage);
    }
    return(no_place);
}

//Converts a place to a string
char * placeToString(PlaceType place) {
    switch (place) {
        case slip:
            return("slip");
        case land:
            return("land");
        case trailor:
            return("trailor");
        case storage:
            return("storage");
        case no_place:
            return("no_place");
        default:
            printf("Invalid Place\n");
            exit(EXIT_FAILURE);
            break;
    }
}

//Comparison function for qsort
int compare(const void *ptr1, const void *ptr2){
  Boat *boat_1 = *(Boat **)ptr1;
  Boat *boat_2 = *(Boat **)ptr2;

  //Handle NULL pointers
  if (!boat_1 && !boat_2) return 0;
  if (!boat_1) return 1;
  if (!boat_2) return -1;
  
  return strcasecmp(boat_1->name, boat_2->name);
}

//Sorts the marina array by the alphabetical order of the boat names
void qSortBoats() {
  qsort(marina, MAX_BOATS, sizeof(Boat*), compare);
}

//Outputs every boat in the marina
void printInventory() {
  qSortBoats();
  for (int i = 0; i < MAX_BOATS; i++) {
    if (marina[i]) {
      Boat *boat = marina[i];
      char extra_string[7];
      if (boat->place == slip) {
	sprintf(extra_string, "# %d", boat->extra.slip_num);
      } else if (boat->place == land) {
	sprintf(extra_string, "  %c", boat->extra.bay_letter);
      } else if (boat->place == trailor) {
	sprintf(extra_string, "%s", boat->extra.license_tag);
      } else if (boat->place == storage) {
	sprintf(extra_string, "# %d", boat->extra.storage_num);
      }
      printf("%-20s %3d' %8s %6s   Owes $%.2f\n", boat->name, boat->length,
	     placeToString(boat->place), extra_string, boat->amount_owed);
    }
  }
}


//Creates a boat struct from a csv line
void addBoat(char * buffer) {
  //Checks for a valid csv line
  if (strlen(buffer) == 0) {
    return;
  }
  if (buffer[0] == '\0') {
    return;
  }

  //Removes newline character
  buffer[strcspn(buffer, "\n")] = '\0';

  Boat * new_boat = (Boat *)malloc(sizeof(Boat));
  //Tests malloc errors
  if (!new_boat) {
    fprintf(stderr, "Memory allocation failed!\n");
    exit(EXIT_FAILURE);
}

  //Copies data into boat stucts
  char place_string[8], extra_string[LICENSE_LENGTH];
  sscanf(buffer, "%[^,],%d,%[^,],%[^,],%f", new_boat->name, &new_boat->length, place_string, extra_string, &new_boat->amount_owed);
  //Handles place variable
  new_boat->place = stringToPlaceType(place_string);
  //Handles extra variable
  if (new_boat->place == slip) {
            new_boat->extra.slip_num = atoi(extra_string);
        } else if (new_boat->place == land) {
            new_boat->extra.bay_letter = extra_string[0];
        } else if (new_boat->place == trailor) {
            strcpy(new_boat->extra.license_tag, extra_string); 
        } else if (new_boat->place == storage) {
            new_boat->extra.storage_num = atoi(extra_string);
        }

  //Adds boat to an empty spot in the marina
  for (int i = 0; i < MAX_BOATS; i++) {
    if (!marina[i]) {
      marina[i] = new_boat;
      return;
    }
  }
}

//Loads the file data into the Marina array
void loadData(char * filename) {
  FILE * file = fopen(filename, "r");
  
  //Tests file open
  if (!file) {
    printf("Error: File won't open\n");
    return;
  }

  //Adds every line in the file
  int boat_num = 0;
  char buffer[256];
  while ((boat_num < MAX_BOATS) && (fgets(buffer, sizeof(buffer), file) != NULL)) {
    addBoat(buffer);
    boat_num++;
  }
}
  
//Removes the boat from the marina
void removeBoat(char * name) {
  //Removes newline character
  name[strcspn(name, "\n")] = '\0';
  
  //Search for boat
  for (int i = 0; i < MAX_BOATS; i++) {
    if (marina[i] && (strcasecmp(marina[i]->name, name) == 0)) {
      //Remove boat from marina
      free(marina[i]);
      
      //Shift marina
      for (int j = i; j < MAX_BOATS - 1; j++) {
	marina[j] = marina[j + 1];
      }
      //Set the pointer added to NULL
      marina[MAX_BOATS - 1] = NULL;
      return;
    }
  }
  //Boat wasn't found
  printf("No boat with that name\n");
}

//Reduces the amount owed for the boat
void payment(char * name) {
  //Removes newline character
  name[strcspn(name, "\n")] = '\0';
  
  //Search for boat
  for (int i = 0; i < MAX_BOATS; i++) {
    if (marina[i] && strcasecmp(marina[i]->name, name) == 0) {
      char buffer[256];
      printf("Please enter the amount to be paid                     : ");
      fgets(buffer, sizeof(buffer), stdin);
      float amount = atof(buffer);
      //Check for a valid payment amount
      if (amount > marina[i]->amount_owed) {
	printf("That is more than the amount owed, $%.2f\n", marina[i]->amount_owed);
	return;
      }
      
      // Deduct the payment from the amount owed
      marina[i]->amount_owed -= amount;
      return;
    }
  }
    //Boat wasn't found
    printf("No boat with that name\n");
}

//Updates the amount owed on each boat for the new month
void updateBalances() {
  for (int i = 0; i < MAX_BOATS; i++) {
    if (marina[i]) {
      double charge = 0.0;
      //Determine the rate from the boat's place
      switch (marina[i]->place) {
      case slip:
	charge = 12.50;
	break;
      case land:
        charge = 14.00;
	break;
      case trailor:
        charge = 25.00;
	break;
      case storage:
        charge = 11.20;
	break;
      default:
	continue;
      }
      //Adds the charge to the amount owed
      marina[i]->amount_owed += marina[i]->length * charge;
    }
  }
}

//Saves data to the file and closes it
void saveMarina(char * filename) {
  FILE * file = fopen(filename, "w");
  
  //Tests file close
  if (!file) {
    printf("Error: Can't close/write to file\n");
    return;
  }

  //Write each boat to the file
  for (int i = 0; i < MAX_BOATS; i++) {
    if (marina[i]) {
      Boat *boat = marina[i];
      char extra_string[LICENSE_LENGTH];
      if (boat->place == slip) {
	sprintf(extra_string, "%d", boat->extra.slip_num);
      } else if (boat->place == land) {
	sprintf(extra_string, "%c", boat->extra.bay_letter);
      } else if (boat->place == trailor) {
	sprintf(extra_string, "%s", boat->extra.license_tag);
      } else if (boat->place == storage) {
	sprintf(extra_string, "%d", boat->extra.storage_num);
      }
      fprintf(file, "%s,%d,%s,%s,%.2f\n", boat->name, boat->length, placeToString(boat->place), extra_string, boat->amount_owed);
    }
  }
  fclose(file);
}
 
//Main function
int main(int argc, char * argv[]) {
  //Checks number of command line args
  if (argc != 2) {
    printf("Error: There should be 1 command line argument\n");
    return 1;
  }
  printf("Welcome to the Boat Management System\n-------------------------------------\n");
  
  loadData(argv[1]);

  //Offers a menu of options
  char option;
  char buffer[256];
  while (1) {
    printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
    fgets(buffer, sizeof(buffer), stdin);
    option = tolower(buffer[0]);

    //Handles user input
    switch (option) {
      case 'i':
        printInventory();
        break;
      case 'a':
	printf("Please enter the boat data in CSV format               : ");
	fgets(buffer, sizeof(buffer), stdin);
	addBoat(buffer);
	break;
      case 'r': 
	printf("Please enter the boat name                             : ");
	fgets(buffer, sizeof(buffer), stdin);
	removeBoat(buffer);
	break;
      case 'p':
	printf("Please enter the boat name                             : ");
	fgets(buffer, sizeof(buffer), stdin);
	char name[NAME_LENGTH];
	strcpy(name, buffer);
	payment(name  );
	break;
      case 'm':
	updateBalances();
	break;
      case 'x': 
	saveMarina(argv[1]);
	//Free all memory on exit
	for (int i = 0; i < MAX_BOATS; i++) {
	  if (marina[i]) {
	    free(marina[i]);
	  }
	}	
	printf("\nExiting the Boat Management System\n");
	return 0;
      default:
	printf("Invalid option %c\n", option);
	break;
    }
  }
}
