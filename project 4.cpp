#include <iostream> // for cout
#include <iomanip> // for setw()
#include <fstream> // for ifstream

using namespace std;

struct NODE { // create a doubly linked list
	double data;

	NODE *next, *prev;
};

struct LIST { // holds data for each list
	int listLen, maxLen, minLen;
	double avg, max, median, min;

	NODE *head, *tail;
};

struct TOTAL { // holds data for all lists
	int mostLen, leastLen;
	int maxLen, minLen;
	int numArray;

	int *most, *least;
	int *max, *min;
	int *arAvg, *arMax, *arMin, *arMed;

	double medAvg, medMax, medMin;
};

// called in main
void createList(TOTAL&, LIST*&);
void calcOutput(TOTAL&, LIST*&);
void printOutput(TOTAL,LIST*);

// sort array
void bubblesort(int*&, TOTAL, LIST*, int);
void selectionsort(int*&, TOTAL, LIST*, int);
void insertionsort(int*&, TOTAL, LIST*, int);
void quicksort(int*&, int, int, LIST*, int);
int partition(int*&, int, int, LIST*, int);
double getVal(int*, int, LIST*, int);

// sort linked list
void mergesort(NODE*&, NODE*&);
void merge(NODE*,NODE*,NODE*,NODE*,NODE*&,NODE*&);
NODE* findMid(NODE*);

// get output
void findMost(TOTAL&, LIST*);
void findLeast(TOTAL&, LIST*);
void findMax(TOTAL&, LIST*);
void findMin(TOTAL&, LIST*);

// helper for output
int findMaxLen(NODE*);
int findMinLen(NODE*);
double findMedian(NODE*);
double findMedian(int*, TOTAL, LIST*, int);

int main(int argc, char** argv)
{
	int i;

	LIST *list;
	TOTAL t;

	createList(t,list);	
	calcOutput(t,list);
	printOutput(t,list);

	cin>>i;
	return 0;
}

void createList(TOTAL &t, LIST* &list)
{
	int i, listNum;
	double listVal;
	NODE *tmp;

	ifstream read("data.txt");

	read>>t.numArray;	list = new LIST[t.numArray];
	for(i = 0; i < t.numArray; i++)
	{
		list[i].tail = NULL;
		list[i].listLen = 0;
		list[i].avg = 0.0; // sum
	}

	while(read>>listNum>>listVal)
	{
		tmp = new NODE;

		if(list[listNum].tail == NULL)
		{
			list[listNum].head = tmp;
			list[listNum].tail = tmp;
			tmp->prev = NULL;
		}
		else
		{
			list[listNum].tail->next = tmp;
			tmp->prev = list[listNum].tail;
			list[listNum].tail = tmp;
		}

		list[listNum].tail->data = listVal;
		list[listNum].tail->next = NULL;
		list[listNum].avg += listVal;
		list[listNum].listLen++;
	}

	list[i].avg /= list[i].listLen;

	return;
}

void calcOutput(TOTAL &t, LIST* &list)
{
	int i;

	t.most = new int[t.numArray];
	t.least = new int[t.numArray];
	t.max = new int[t.numArray];
	t.min = new int[t.numArray];

	t.arAvg = new int[t.numArray];
	t.arMax = new int[t.numArray];
	t.arMin = new int[t.numArray];
	t.arMed = new int[t.numArray];

	findMost(t,list);
	findLeast(t,list);

	for(i = 0; i < t.numArray; i++)
	{
		mergesort(list[i].head,list[i].tail);
		list[i].min = list[i].head->data;		
		list[i].max = list[i].tail->data;
		list[i].minLen = findMinLen(list[i].head);
		list[i].maxLen = findMaxLen(list[i].tail);
		list[i].median = findMedian(list[i].head);
	} 

	findMax(t,list);
	findMin(t,list);

	for(i = 0; i < t.numArray; i++)
	{
		t.arMax[i] = i;
		t.arMin[i] = i;
		t.arAvg[i] = i;
		t.arMed[i] = i;
	}

	bubblesort(t.arMax,t,list,2);
	t.medMax = findMedian(t.arMax,t,list,2);

	selectionsort(t.arMin,t,list,3);
	t.medMin = findMedian(t.arMin,t,list,3);

	insertionsort(t.arAvg,t,list,1);
	t.medAvg = findMedian(t.arAvg,t,list,1);

	quicksort(t.arMed,0,t.numArray-1,list,4);

	return;
}

void printOutput(TOTAL t, LIST* list)
{
	int i, input;
	NODE *curr;

	cout<<"======="<<endl<<"Output:"<<endl<<"======="<<endl<<endl;
	
	// Most Values
	cout<<"List";
	if(t.mostLen)
	{
		cout<<"s ";
		for(i = 0; i <= t.mostLen; i++)
		{
			cout<<t.most[i];
			if(i != t.mostLen) cout<<",";
		}
	}
	else
		cout<<" "<<t.most[0];
	cout<<" had the most values with "<<list[t.most[0]].listLen<<"."<<endl;
	cout<<endl;

	// Least Values
	cout<<"List";
	if(t.leastLen)
	{
		cout<<"s ";
		for(i = 0; i <= t.leastLen; i++)
		{
			cout<<t.least[i];
			if(i != t.leastLen) cout<<",";
		}
	}
	else
		cout<<" "<<t.least[0];
	cout<<" had the least values with "<<list[t.least[0]].listLen<<"."<<endl;
	cout<<endl;

	// Max Values
	cout<<"List";
	if(t.maxLen)
	{
		cout<<"s ";
		for(i = 0; i <= t.maxLen; i++)
		{
			cout<<t.max[i];
			if(i != t.maxLen) cout<<",";
		}
	}
	else
		cout<<" "<<t.max[0];
	cout<<" had the highest value, which was "<<list[t.max[0]].max<<"."<<endl;
	for(i = 0; i <= t.maxLen; i++)
		cout<<"This value appears "<<list[t.max[i]].maxLen<<" times in list "<<t.max[i]<<endl;
	cout<<endl;

	// Min Values
	cout<<"List";
	if(t.minLen)
	{
		cout<<"s ";
		for(i = 0; i <= t.minLen; i++)
		{
			cout<<t.min[i];
			if(i != t.minLen) cout<<",";
		}
	}
	else
		cout<<" "<<t.min[0];
	cout<<" had the lowest value, which was "<<list[t.min[0]].min<<"."<<endl;
	for(i = 0; i <= t.maxLen; i++)
		cout<<"This value appears "<<list[t.min[i]].minLen<<" times in list "<<t.min[i]<<endl;
	cout<<endl;

	// Avg, max, and min values for each list
	cout<<right<<setw(4)<<"List"<<setw(14)<<"Max"<<setw(14)<<"Min"<<setw(14)<<"Avg"<<setw(14)<<"Length"<<endl;
	cout<<"============================================================"<<endl;
	for(i = 0; i < t.numArray; i++)
	{
		cout<<right
			<<setw(4)<<i
			<<setw(14)<<list[i].max
			<<setw(14)<<list[i].min
			<<setw(14)<<list[i].avg
			<<setw(14)<<list[i].listLen
			<<endl;
	}
	cout<<endl; 

	// Median user input
	cout<<"Enter category number (median):";
	cin>>input;
	cout<<"The median value for list "<<input<<" is "<<list[input].median<<endl;

	// Ascending user input
	cout<<"Enter category number (ascending):";
	cin>>input;
	cout<<"List["<<input<<"]"<<endl;
	for(curr = list[input].head; curr != list[input].tail; curr = curr->next)
		cout<<curr->data<<endl;

	// Descending user input
	cout<<"Enter category number (descending):";
	cin>>input;
	cout<<"List["<<input<<"]"<<endl;
	for(curr = list[input].tail; curr != list[input].head; curr = curr->prev)
		cout<<curr->data<<endl;

	cout<<"arMax median = "<<t.medMax<<endl;
	cout<<"arMin median = "<<t.medMin<<endl;
	cout<<"arAvg median = "<<t.medAvg<<endl;
	cout<<endl;

	cout<<"Category ascending based on avg:"<<endl;
	for(i = 0; i < t.numArray; i++)
		cout<<i<<"\t"<<t.arAvg[i]<<"\t"<<list[t.arAvg[i]].avg<<endl;
	cout<<endl;

	cout<<"Category descending based on med:"<<endl;
	for(i = t.numArray-1; i >= 0; i--)
		cout<<i<<"\t"<<t.arMed[i]<<"\t"<<list[t.arMed[i]].median<<endl;
	cout<<endl;

	return;
}

void bubblesort(int* &ar, TOTAL t, LIST* list, int choice)
{
	int count, tmp;
	bool swap;
	double ac, acp1;

	do
	{
		swap = false;
		for(count = 0; count < (t.numArray - 1); count++)
		{
			ac = getVal(ar,count,list,choice);
			acp1 = getVal(ar,count+1,list,choice);
			if(ac > acp1)
			{
				tmp = ar[count];
				ar[count] = ar[count+1];
				ar[count+1] = tmp;

				swap = true;
			}
		}
	} while(swap);

	return;
}

void selectionsort(int* &ar, TOTAL t, LIST *list, int choice)
{
	int startScan, minIndex, index;
	int minValue;
	double mv, ai;

	for(startScan = 0; startScan < (t.numArray - 1); startScan++)
	{
		minIndex = startScan;
		minValue = ar[startScan];
		for(index = startScan + 1; index < t.numArray; index++)
		{
			ai = getVal(ar,index,list,choice);
			mv = getVal(ar,minIndex,list,choice);
			if(ai < mv)
			{
				minValue = ar[index];
				minIndex = index;
			}
		}
		ar[minIndex] = ar[startScan];
		ar[startScan] = minValue;
	}

	return;
}

void insertionsort(int* &ar, TOTAL t, LIST *list,  int choice)
{
	int j;
	int i;
	int key;
	double iv, kv;

	for(j = 1; j < t.numArray; j++)
	{
		key = ar[j];
		kv = getVal(ar,j,list,choice);

		i = j - 1;
		iv = getVal(ar,i,list,choice);	

		while((i >= 0) && (iv > kv))
		{
			ar[i+1] = ar[i];

			--i;
			if(i >= 0) 			
				iv = getVal(ar,i,list,choice);
		}
		ar[i+1] = key;
	}

	return;
}

void quicksort(int*& ar, int p, int r, LIST* list, int choice)
{
	int q;

	if(p < r)
	{
		q = partition(ar,p,r,list,choice);
		quicksort(ar,p,q-1,list,choice);
		quicksort(ar,q+1,r,list,choice);
	}

	return;
}

int partition(int*& ar, int p, int r, LIST* list, int choice)
{
	int i, j, tmp;
	double x, aj;

	x = getVal(ar,r,list,choice);
	i = p - 1;
	for(j = p; j <= r-1; j++)
	{
		aj = getVal(ar,j,list,choice);
		if(aj <= x)
		{
			i++;

			tmp = ar[i];
			ar[i] = ar[j];
			ar[j] = tmp;
		}
	}
	tmp = ar[i+1];
	ar[i+1] = ar[r];
	ar[r] = tmp;

	return ++i;
}

double getVal(int *ar, int pos, LIST *list, int choice)
{
	double ret;

	switch(choice) {
		case 1: // avg
			ret = list[ar[pos]].avg;
			break;
		case 2: // max
			ret = list[ar[pos]].max;
			break;
		case 3: // min
			ret = list[ar[pos]].min;
			break;
		case 4: // med
			ret = list[ar[pos]].median;
			break;
		}

	return ret;
}

void mergesort(NODE* &head, NODE* &tail)
{
	NODE *h2, *t2, *h3, *t3;

	// base case
	if((NULL == head) || (NULL == head->next))
		return;

	// find mid
	h2 = findMid(head);

	// divide
	t2 = tail;
	tail = h2->prev;
	h2->prev = NULL;
	tail->next = NULL;

	mergesort(head,tail);
	mergesort(h2,t2);

	merge(head,tail,h2,t2,h3,t3);
	head = h3;
	tail = t3;

	return;
}

void merge(NODE *h1,NODE *t1,NODE *h2,NODE *t2,NODE *&h3,NODE *&t3)
{
	if(NULL == h1)
	{
		h3 = h2;
		t3 = t2;
		return;
	}
	if(NULL == h2)
	{
		h3 = h1;
		t3 = t1;
		return;
	}
	if(h1->data < h2->data)
	{
		h3 = h1;
		merge(h1->next,t1,h2,t2,h3->next,t3);
		h3->next->prev = h1;
	}
	else
	{
		h3 = h2;
		merge(h1,t1,h2->next,t2,h3->next,t3);
		h3->next->prev = h2;
	}

	return;
}

NODE* findMid(NODE *head)
{
	NODE *h1 = head;
	NODE *h2 = head->next;

	while((h2 != NULL) && (h2->next != NULL))
	{
		h1 = h1->next;
		h2 = h2->next->next;
	}
	h2 = h1->next;

	return h2;
}

void findMost(TOTAL &t, LIST* list)
{
	int i;

	t.mostLen = 0; 
	t.most[0] = 0;

	for(i = 0; i < t.numArray; i++)
	{
		if(list[i].listLen > list[t.most[t.mostLen]].listLen) // new longest list
		{
			t.mostLen = 0; // reset array length/counter
			t.most[0] = i; // add list index to array
		}
		else if(list[i].listLen == list[t.most[t.mostLen]].listLen) // duplicate longest list
		{
			t.mostLen++; // increment length
			t.most[t.mostLen] = i; // add to array
		}
	}

	return;
}

void findLeast(TOTAL &t, LIST* list)
{
	int i;

	t.leastLen = 0; 
	t.least[0] = 0;

	for(i = 0; i < t.numArray; i++)
	{
		if(list[i].listLen < list[t.least[t.leastLen]].listLen) // new longest list
		{
			t.leastLen = 0; // reset array length/counter
			t.least[0] = i; // add list index to array
		}
		else if(list[i].listLen == list[t.least[t.leastLen]].listLen) // duplicate longest list
		{
			t.leastLen++; // increment length
			t.least[t.leastLen] = i; // add to array
		}
	}

	return;
}

void findMax(TOTAL &t, LIST* list)
{
	int i;

	t.maxLen = 0;
	t.max[0] = 0;

	for(i = 0; i < t.numArray; i++)
	{
		// Find max values
		if(list[i].max > list[t.max[t.maxLen]].max)
		{
			t.maxLen = 0;
			t.max[0] = i;
		}
		else if(list[i].max == list[t.max[t.maxLen]].max)
		{
			t.maxLen++;
			t.max[t.maxLen] = i;
		}
	}

	return;
}

void findMin(TOTAL &t, LIST* list)
{
	int i;

	t.minLen = 0;
	t.min[0] = 0;

	for(i = 0; i < t.numArray; i++)
	{
		// Find max values
		if(list[i].min < list[t.min[t.minLen]].min)
		{
			t.minLen = 0;
			t.min[0] = i;
		}
		else if(list[i].min == list[t.min[t.minLen]].min)
		{
			t.minLen++;
			t.min[t.minLen] = i;
		}
	}

	return;
}

int findMaxLen(NODE *tail)
{
	int len = 1;
	NODE *curr = tail;

	while(curr->data == curr->prev->data)
	{
		len++;
		curr = curr->next;
	}

	return len;
}

int findMinLen(NODE *head)
{
	int len = 1;
	NODE *curr = head;

	while(curr->data == curr->next->data)
	{
		len++;
		curr = curr->next;
	}

	return len;
}

double findMedian(NODE *head)
{
	double med;

	NODE *h1 = head;
	NODE *h2 = head->next;

	while((h2 != NULL) && (h2->next != NULL))
	{
		h1 = h1->next;
		h2 = h2->next->next;
	}
	if(h2 == NULL)
		med = h1->data;
	else
	{
		h2 = h1->next;
		med = (h1->data + h2->data)/2.0;
	}

	return med;
}

double findMedian(int *ar, TOTAL t, LIST* list, int choice) // not done
{
	int index;
	double ret, ai, aip1;

	index = (t.numArray/2) - 1;
	ai = getVal(ar,index,list,choice);
	aip1 = getVal(ar,index+1,list,choice);
	if(t.numArray%2 != 0)
		ret = aip1;
	else
		ret = (ai + aip1)/2.0;

	return ret;
}