#include <iostream>
#include <pthread.h>

using namespace std;



//Test sample (from Question statement example)
int puzzle[9][9] = {{6,2,4,5,3,4,1,8,7},
					{5,1,9,7,2,8,6,3,4},
					{8,3,7,6,1,4,2,9,5},
					{1,4,3,8,6,5,7,2,9},
					{9,5,8,2,4,7,3,6,1},
					{7,6,2,3,9,1,4,5,8},
					{3,7,1,9,5,6,8,4,2},
					{4,9,6,1,8,2,5,7,3},
					{2,8,5,4,7,3,9,1,6}
				   };
				   


//Structure which holds values for parameters passed to threads
struct parameters
{
	int row;
	int column;
	
};

//Structure which holds data about invalid entries
class ret
{
	public:
	
	pthread_t id;	//thread id
	int row;		//row of invalid entry
	int col;		//col of invalid entry
	bool repeat;	//If the error is repeating value, set to true
	bool out_of_range;	//If the error is out of range, set to true
	int row_or_col_check;	//If row checker thread found error, set to 0
							//If col checker thread found error, set to 1
							//Default is -1 which means that a 3x3 grid searching thread found error
	
	ret* next;	//points to next object
	
	//contructor
	ret()
	{
		id = 0;
		row_or_col_check = -1; //Not set (0 = rowCheck, 1 = colCheck, -1 = gridCheck)
		row = 0;
		col = 0;
		repeat = false;
		out_of_range = false;
		next = NULL;
	}
		
};

//Structure for linked list of invalid entries
class invalid_list
{
	public:
		
	ret *head;	//points to head of linked list
	int numOfEntries;	//Total number of invalid entries
	
	//Constructor
	invalid_list()
	{
		head = NULL;
		numOfEntries = 0;
	}
	
	//Destructor
	~invalid_list()
	{
		ret* temp = head;
		
		while(temp != NULL)
		{
			ret* prev = temp;
			temp = temp->next;
			delete prev;		//Deallocation
		}
	}
	
	//insert new invalid entry data
	void insertEntry(ret* r)
	{
		bool addNum = true; //if true, increment total entries
		
		//If no node in the list
		if(head == NULL)
		{
			head = r;
			numOfEntries++;
			return;
		}
		
		//Else
		
		ret* temp = head;
		
		while(temp->next != NULL)
		{
			//If index already exists as part of some data, do not increment number of entries
			if(temp->row == r->row && temp->col == r->col)
			{
				addNum = false;
			}
			
			//next node
			temp = temp->next;
		}
		
		if(temp->row == r->row && temp->col == r->col)
		{
			addNum = false;
		}
			
		temp->next = r; //adding to list
		
		if(addNum) //If entry is not already mentioned in the list, then increment numOfEntries
		{
			numOfEntries++;
		}
		
		return;
	}
	
	void displayList()
	{
		ret* temp = head;
		int i = 1;
		
		cout << "Total invalid entries: " << numOfEntries << endl << endl;
		cout << "--------DETAILS-------" << endl << endl;
		
		while(temp != NULL)
		{
		
			cout << "[" << i << "] \n\tThread ID: "<< temp->id << "\n\tIndex: [" << temp->row+1 << "][" << temp->col+1 << "]\n\t";
			cout << "Value: " << puzzle[temp->row][temp->col] << "\n\t";
			cout << "Above entry made ";
			
			if(temp->row_or_col_check == 0) //If Row Check
			{
				cout << "Row [" << temp->row+1 << "] Invalid\n"; 
			}
			else if(temp->row_or_col_check == 1) //If Col Check
			{
				cout << "Column [" << temp->col+1 << "] Invalid\n";
			}
			else	//Invalid Grid
			{
				if(temp->row < 3 && temp->col < 3)
				{
					cout << "Grid [1] Invalid\n";
				}
				else if(temp->row < 3 && temp->col < 6)
				{
					cout << "Grid [2] Invalid\n";
				}
				else if(temp->row < 3 && temp->col < 9)
				{
					cout << "Grid [3] Invalid\n";
				}
				else if(temp->row < 6 && temp->col < 3)
				{
					cout << "Grid [4] Invalid\n";
				}
				else if(temp->row < 6 && temp->col < 6)
				{
					cout << "Grid [5] Invalid\n";
				}
				else if(temp->row < 6 && temp->col < 9)
				{
					cout << "Grid [6] Invalid\n";
				}
				else if(temp->row < 9 && temp->col < 3)
				{
					cout << "Grid [7] Invalid\n";
				}
				else if(temp->row < 9 && temp->col < 6)
				{
					cout << "Grid [8] Invalid\n";
				}
				else if(temp->row < 9 && temp->col < 9)
				{
					cout << "Grid [9] Invalid\n";
				}
			}
			
			cout << "\t";
			
			if(temp->repeat)
			{
				cout << "Reason: value " << puzzle[temp->row][temp->col] << " is repeating\n";
			}
			
			if(temp->out_of_range)
			{
				cout << "Reason: value " << puzzle[temp->row][temp->col] << " is outside the range of 1 to 9 (inclusive)\n";
			}

			cout << endl;
			
			i++;
			temp = temp->next;
			
		}
	}
};

//Global list of invalid entries
invalid_list* InvList = new invalid_list;


//Thread functions prototypes
void* rowCheck(void*);	//Row searching
void* colCheck(void*);	//Col searching
void* gridCheck(void*);	//Grid searching

//Utility Functions
void printPuzzle();		//Prints the puzzle in a matrix format

//Threads and Mutex
pthread_mutex_t m1;	//mutex for locking shared data

pthread_t rowThread;	//thread for row searching
pthread_t colThread;	//thread for col searching

//threads for grid searching (9 (3x3) grids = 9 threads)
pthread_t Grid_1_Thread,Grid_2_Thread,Grid_3_Thread,Grid_4_Thread,Grid_5_Thread,Grid_6_Thread,Grid_7_Thread,Grid_8_Thread,Grid_9_Thread;


//================================DRIVER FUNCTION================================


int main()
{
	
	//================Parameters for threads processing rows and cols=================
	
		//Rows
		parameters *rowParam = new parameters;
		rowParam->row = 0;
		rowParam->column = 0;
		
		//Cols
		parameters *colParam = new parameters;
		colParam->row = 0;
		colParam->column = 0;
	
	
	//================Parameters for threads processing 3x3 Grids=================
	
		//Grid 1
		parameters *GridParam1 =  new parameters;
		GridParam1->row = 0;
		GridParam1->column = 0;
		
		//Grid 2
		parameters *GridParam2 =  new parameters;
		GridParam2->row = 0;
		GridParam2->column = 3;
		
		//Grid 3
		parameters *GridParam3 =  new parameters;
		GridParam3->row = 0;
		GridParam3->column = 6;
		
		//Grid 4
		parameters *GridParam4 =  new parameters;
		GridParam4->row = 3;
		GridParam4->column = 0;
		
		//Grid 5
		parameters *GridParam5 =  new parameters;
		GridParam5->row = 3;
		GridParam5->column = 3;
		
		//Grid 6
		parameters *GridParam6 =  new parameters;
		GridParam6->row = 3;
		GridParam6->column = 6;
		
		//Grid 7
		parameters *GridParam7 =  new parameters;
		GridParam7->row = 6;
		GridParam7->column = 0;
		
		//Grid 8
		parameters *GridParam8 =  new parameters;
		GridParam8->row = 6;
		GridParam8->column = 3;
		
		//Grid 9
		parameters *GridParam9 =  new parameters;
		GridParam9->row = 6;
		GridParam9->column = 6;
		
		
	//==============Creating threads==============
		
		//For storing return values
		int* rowVal;
		int* colVal;
		int* Grid_1_Val,*Grid_2_Val,*Grid_3_Val,*Grid_4_Val,*Grid_5_Val,*Grid_6_Val,*Grid_7_Val,*Grid_8_Val,*Grid_9_Val;
		
		//Initializing mutex
		pthread_mutex_init(&m1 ,NULL);
		
		//Creating threads
		pthread_create(&rowThread,NULL,rowCheck,(void*)rowParam);			//Row search
		pthread_create(&colThread,NULL,colCheck,(void*)colParam);			//Col search
		pthread_create(&Grid_1_Thread,NULL,gridCheck,(void*)GridParam1);	//Grid 1 Search
		pthread_create(&Grid_2_Thread,NULL,gridCheck,(void*)GridParam2);	//Grid 2 Search
		pthread_create(&Grid_3_Thread,NULL,gridCheck,(void*)GridParam3);	//Grid 3 Search
		pthread_create(&Grid_4_Thread,NULL,gridCheck,(void*)GridParam4);	//Grid 4 Search
		pthread_create(&Grid_5_Thread,NULL,gridCheck,(void*)GridParam5);	//Grid 5 Search
		pthread_create(&Grid_6_Thread,NULL,gridCheck,(void*)GridParam6);	//Grid 6 Search
		pthread_create(&Grid_7_Thread,NULL,gridCheck,(void*)GridParam7);	//Grid 7 Search
		pthread_create(&Grid_8_Thread,NULL,gridCheck,(void*)GridParam8);	//Grid 8 Search
		pthread_create(&Grid_9_Thread,NULL,gridCheck,(void*)GridParam9);	//Grid 9 Search
	
		//Waiting for threads to complete
		pthread_join(rowThread,(void**)&rowVal);
		pthread_join(colThread,(void**)&colVal);
		pthread_join(Grid_1_Thread,(void**)&Grid_1_Val);
		pthread_join(Grid_2_Thread,(void**)&Grid_2_Val);
		pthread_join(Grid_3_Thread,(void**)&Grid_3_Val);
		pthread_join(Grid_4_Thread,(void**)&Grid_4_Val);
		pthread_join(Grid_5_Thread,(void**)&Grid_5_Val);
		pthread_join(Grid_6_Thread,(void**)&Grid_6_Val);
		pthread_join(Grid_7_Thread,(void**)&Grid_7_Val);
		pthread_join(Grid_8_Thread,(void**)&Grid_8_Val);
		pthread_join(Grid_9_Thread,(void**)&Grid_9_Val);
				
	
		
		
	//=======================Printing Findings (Output)==========================
	
		//Printing puzzle
		cout << "===================PUZZLE INPUT==================" << endl << endl;
		printPuzzle();
	
		cout << "\n\n";
		cout << "======================REPORT=====================" << endl << endl;
	
					
		//If all threads return 1 (Valid)
		if(1 == (*rowVal)*(*colVal)*(*Grid_1_Val)*(*Grid_2_Val)*(*Grid_3_Val)*(*Grid_4_Val)*(*Grid_5_Val)*(*Grid_6_Val)*(*Grid_7_Val)*(*Grid_8_Val)*(*Grid_9_Val))
		{
			cout << "<---PUZZLE IS VALID--->" << endl;
		}
		else //Invalid case
		{
			cout << "<---PUZZLE IS INVALID--->" << endl << endl;
			
			InvList->displayList();
		}	

		pthread_mutex_destroy(&m1);
		pthread_exit(NULL);
	
}




//===================Function for row searching===================

void* rowCheck(void* arg)
{
	parameters *p = (parameters*) arg; //Accessing arguments
	
	//Function return value
	int* rVal = new int;
	*rVal = 1; //Valid by default
	
	//Invalid entry object
	ret* r = new ret;
	r->id = pthread_self();	//thread id
	r->row_or_col_check = 0; //Signifies row operation
	
	
	for(int i = p->row; i < 9; i++)
	{
		//rowFlag[i] = True, if found number i in row, else remains false
		// 	 			  {col1, col2, col3, col4, col5, col6, col7, col8, col9}
		bool rowFlag[9] = {false,false,false,false,false,false,false,false,false};
		
		r->row = i; //setting current row number
		
		for(int j = p->column; j < 9; j++)
		{
			r->col = j; //setting current col number
			
			
			if(puzzle[i][j] < 1 || puzzle[i][j] > 9)
			{

				r->out_of_range = true; //Value is out of range
				*rVal = 0; //Invalid
				
				//Critical section
				pthread_mutex_lock(&m1);
				
					InvList->insertEntry(r);	//Adding to invalid entry list
				
				pthread_mutex_unlock(&m1);
				//Critical section finished
				
				//After storing invaid entry, a new object is created in case another invalid entry is found
				r = new ret;
				r->id = pthread_self();	
				r->row_or_col_check = 0; //Signifies row operation
				r->row = i; //setting current row number
			}
			
			else if(rowFlag[puzzle[i][j]-1] == false)
			{
				rowFlag[puzzle[i][j]-1] = true;	//Valid case
			}
			else	//If number has already been found, the row is invalid since the number repeats
			{
				r->repeat = true; //Value is repeating
				*rVal = 0; //Invalid
				
				//Critical section
				pthread_mutex_lock(&m1);
				
					InvList->insertEntry(r);	//Adding to invalid entry list
				
				pthread_mutex_unlock(&m1);
				//Critical section finished
				
				//After storing invaid entry, a new object is created in case another invalid entry is found
				r = new ret;
				r->id = pthread_self();	
				r->row_or_col_check = 0; //Signifies row operation
			}
		}
		
	}
	
	//If current object is valid, then do not add to list and deallocate space
	if(!r->out_of_range && !r->repeat)
	{
		delete r;
	}
	
	//exit
	pthread_exit((void*)rVal);
}


//===================Function for col searching===================

void* colCheck(void* arg)
{
	parameters *p = (parameters*) arg; //Accessing arguments
	
	//Function return value
	int* rVal = new int;
	*rVal = 1; //Valid by default
	
	//Invalid entry object
	ret* r = new ret;
	r->id = pthread_self();	//thread id
	r->row_or_col_check = 1; //Signifies col operation
	
	for(int i = p->column; i < 9; i++)
	{
		//colFlag[i] = True, if found number i in col, else remains false
		// 	 			  {row1, row2, row3, row4, row5, row6, row7, row8, row9}
		bool colFlag[9] = {false,false,false,false,false,false,false,false,false};
		
		r->col = i; //setting current col number		
		
		for(int j = p->row; j < 9; j++)
		{
			r->row = j; //setting current row number
			
			if(puzzle[j][i] < 1 || puzzle[j][i] > 9)
			{
				
				*rVal = 0; //Invalid
				r->out_of_range = true; //Value is out of range
				
				//Critical section
				pthread_mutex_lock(&m1);
				
					InvList->insertEntry(r);	//Adding to invalid entry list
				
				pthread_mutex_unlock(&m1);
				//Critical section finished
				
				//After storing invaid entry, a new object is created in case another invalid entry is found
				r = new ret;
				r->id = pthread_self();	
				r->row_or_col_check = 1; //Signifies col operation
				r->col = i; //setting current col number
			}
			
			else if(colFlag[puzzle[j][i]-1] == false)
			{
				colFlag[puzzle[j][i]-1] = true;	//Valid case
			}
			else	//If number has already been found, the row is invalid since the number repeats
			{
				*rVal = 0; //Invalid
				r->repeat = true; //Value is repeating
				
				//Critical section
				pthread_mutex_lock(&m1);
				
					InvList->insertEntry(r);	//Adding to invalid entry list
				
				pthread_mutex_unlock(&m1);
				//Critical section finished
				
				//After storing invaid entry, a new object is created in case another invalid entry is found
				r = new ret;
				r->id = pthread_self();	
				r->row_or_col_check = 1; //Signifies col operation
			}
		}
		
	}
	
	//If current object is valid, then do not add to list and deallocate space
	if(!r->out_of_range && !r->repeat)
	{
		delete r;
	}
	
	//exit
	pthread_exit((void*)rVal);
}


//===================Function for grid searching===================

void* gridCheck(void* arg)
{
	parameters *p = (parameters*) arg; //Accessing arguments
	
	//Function return value
	int* rVal = new int;
	*rVal = 1; //Valid by default
	
	//Invalid entry object
	ret* r = new ret;
	r->id = pthread_self();	//thread id
	//r->row_or_col_check = -1 signifies grid check (set in constructor)
	
	//gridFlag[i] = True, if found number i in row, else remains false
	// 	 			  {r1c1, r1c2, r1c3, r2c1, r2c2, r2c3, r3c1, r3c2, r3c3}
	bool gridFlag[9] = {false,false,false,false,false,false,false,false,false};

	
	for(int i = p->row; i < p->row + 3; i++)
	{
		r->row = i; //setting current row number		
		
		for(int j = p->column; j < p->column + 3; j++)
		{
			r->col = j; //setting current col number
			
			if(puzzle[i][j] < 1 || puzzle[i][j] > 9)
			{
				*rVal = 0; //Invalid
				r->out_of_range = true; //Value is out of range
				
				//Critical section
				pthread_mutex_lock(&m1);
				
					InvList->insertEntry(r);	//Adding to invalid entry list
				
				pthread_mutex_unlock(&m1);
				//Critical section finished
				
				//After storing invaid entry, a new object is created in case another invalid entry is found
				r = new ret;
				r->id = pthread_self();	
			}
			
			
			else if(gridFlag[puzzle[i][j]-1] == false)
			{
				gridFlag[puzzle[i][j]-1] = true;
			}
			else	//If number has already been found, the row is invalid since the number repeats
			{
				*rVal = 0; //Invalid
				r->repeat = true; //Value is repeating
				
				//Critical section
				pthread_mutex_lock(&m1);
				
					InvList->insertEntry(r);	//Adding to invalid entry list
				
				pthread_mutex_unlock(&m1);
				//Critical section finished
				
				//After storing invaid entry, a new object is created in case another invalid entry is found
				r = new ret;
				r->id = pthread_self();
			}
		}
		
	}
	
	//If current object is valid, then do not add to list and deallocate space
	if(!r->out_of_range && !r->repeat)
	{
		delete r;
	}
	
	//exit
	pthread_exit((void*)rVal);
}


//===================Function for printing puzzle===================

void printPuzzle()		//Prints the puzzle in a matrix format
{
	
	for(int i = 0; i < 9; i++)
	{
		cout << "\t";
		
		for(int j = 0; j < 9; j++)
		{
			cout << puzzle[i][j] << "\t";
		}
		
		cout << "\n\n";
	}	
}



