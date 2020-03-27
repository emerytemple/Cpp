#include <iostream> // for cout
#include <iomanip> // for setw()
#include <fstream> // for ifstream

using namespace std;

template <typename T>
struct NODE {
	T data;
	NODE<T> *next;
};

template <typename T>
struct LIST {
	int listLen, maxLen, minLen;
	double avg, max, median, min;
	NODE<T> *head, *tail;
};

template <typename T>
void createList(LIST<T>*&, int&);
template <typename T>
void initializeList(LIST<T>*, int, double);
template <typename T>
void appendList(LIST<T>*, int, double);
template <typename T>
void printList(LIST<T>*, int);
template <typename T>
NODE<T>* mergesort(NODE<T>*);
template <typename T>
NODE<T>* merge(NODE<T>*, NODE<T>*);
template <typename T>
NODE<T>* findMidAndDivide(NODE<T>*);
template <typename T>
void InsertionSort(T*, int);
template <typename T>
double calcAvg(LIST<T>*, int);


int main(int argc, char** argv)
{
	int i, numArray;
	LIST<double> *list;

	createList<double>(list, numArray);
	printList<double>(list, numArray);

	for(i = 0; i < numArray; i++)
		list[i].head = mergesort(list[i].head);
	printList<double>(list, numArray);

	for(i = 0; i < numArray; i++)
	{	
		list[i].avg = calcAvg(list,i);
		cout<<"i = "<<i<<" avg = "<<list[i].avg<<endl;
	}



	cin>>i;
	return 0;
}

template <typename T>
void createList(LIST<T>* &list, int &numArray)
{
	int i, listNum;
	double listVal;

	ifstream read("data.txt");

	read>>numArray;
	cout<<"numArray = "<<numArray<<endl<<endl;

	// Actually create the lists
	list = new LIST<T>[numArray];
	for(i = 0; i < numArray; i++) list[i].head = NULL;

	while(read>>listNum)
	{
		read>>listVal;
		cout<<listNum<<" "<<listVal<<" "<<endl;

		if(list[listNum].head == NULL) initializeList(list,listNum,listVal);
		else appendList(list,listNum,listVal);
	}

	return;
}

template <typename T>
void initializeList(LIST<T>* list, int listNum, double listVal)
{
	list[listNum].listLen = 1;
	list[listNum].head = new NODE<T>;
	list[listNum].tail = list[listNum].head;
	list[listNum].head->data = listVal;
	list[listNum].head->next = NULL;

	return;
}

template <typename T>
void appendList(LIST<T>* list, int listNum, double listVal)
{
	list[listNum].listLen++;
	list[listNum].tail->next = new NODE<T>;
	list[listNum].tail = list[listNum].tail->next;
	list[listNum].tail->data = listVal;
	list[listNum].tail->next = NULL;

	return;
}

template <typename T>
void printList(LIST<T>* list, int numArray)
{
	int i;
	NODE<T> *curr;

	for(i = 0; i < numArray; i++)
	{
		cout<<"list["<<i<<"]"<<endl;
		cout<<right<<setw(10)<<list[i].head<<setw(10)<<list[i].head->data<<setw(10)<<list[i].head->next<<endl;
		for(curr = list[i].head->next; curr != NULL; curr = curr->next)
		{
			cout<<right<<setw(10)<<curr<<setw(10)<<curr->data<<setw(10)<<curr->next<<endl;
		}
		cout<<right<<setw(10)<<list[i].tail<<endl;
		cout<<endl<<endl;
	}

	return;
}

template <typename T>
NODE<T>* mergesort(NODE<T>* head)
{
	NODE<T>* h2;
	
	if((NULL == head) || (NULL == head->next))
		return head;
	h2 = findMidAndDivide(head);

	return merge(mergesort(head),mergesort(h2));
}

template <typename T>
NODE<T>* merge(NODE<T>* h1, NODE<T>* h2)
{
	NODE<T>* h3;
	if(NULL == h1)
		return h2;
	if(NULL == h2)
		return h1;
	if(h1->data < h2->data)
	{
		h3 = h1;
		h3->next = merge(h1->next,h2);
	}
	else
	{
		h3 = h2;
		h3->next = merge(h1,h2->next);
	}

	return h3;
}

template <typename T>
NODE<T>* findMidAndDivide(NODE<T>* head)
{
	NODE<T> *h1 = head;
	NODE<T> *h2 = head->next;

	while((h2 != NULL) && (h2->next != NULL))
	{
		head = head->next;
		h2 = h2->next->next;
	}

	h2 = head->next;
	head->next = NULL;
	head = h1;

	return h2;
}

template <typename T>
void InsertionSort(T* A, int length)
{
	int j;
	T key;

	for(j = 2:length)
	{
		key = A[j];

		// Insert A[j] into the sorted
		// sequence A[1..j-1]

		i = j - 1;
		while((i > 0) && (A[i] > key))
		{
			A[i+1] = A[i];
			i--;
		}
		A[i+1] = key;
	}
}

template <typename T>
double calcAvg(LIST<T>* list, int category)
{
	double avg = 0.0;
	NODE<T>* curr;

	for(curr = list[category].head; curr != NULL; curr = curr->next)
		avg += curr->data;
	avg /= list[category].listLen;

	return avg;
}