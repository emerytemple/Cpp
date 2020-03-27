#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

// LIST holds data for each list
struct LIST {
	int category;
	int listLen; // length of list
	int maxLen, minLen; // number of times max or min values appear in the list
	double avg, max, median, min;

	vector<double> data; // list values
};

// TOTAL holds data for all lists
struct TOTAL { 
	int numArray; // total number of lists

	int mostLen, leastLen, maxLen, minLen; // number of lists that contain the specified value

	double medAvg, medMax, medMin;

	LIST most, least, max, min; // a list that contains the specified value,
				    // used to get the value itself

	vector<int> arMost, arLeast, arMax, arMin; // array of category numbers
						   // stores all category numbers that contain the specified value
};

// populate LIST struct
vector<LIST> initList(TOTAL&);

// helper for LIST
double listMedian(vector<LIST>, int);

// populate TOTAL struct
void initTotal(TOTAL&, vector<LIST>);

// helper for TOTAL
vector<int> findLen(TOTAL&, vector<LIST>, int&, int);
double totalMedian(TOTAL, vector<LIST>, int);

// STL comp functions
bool lenComp(LIST, LIST);
bool maxComp(LIST, LIST);
bool minComp(LIST, LIST);
bool avgComp(LIST, LIST);
bool medComp(LIST, LIST);

// required output
void printOutput(TOTAL, vector<LIST>);

// helper for output
void printMost(TOTAL, vector<LIST>);
void printLeast(TOTAL, vector<LIST>);
void printMax(TOTAL, vector<LIST>);
void printMin(TOTAL, vector<LIST>);

int main(int argc, char** argv)
{
	vector<LIST> list;
	TOTAL t;

	list = initList(t);
	initTotal(t,list);
	printOutput(t,list);	

	return 0;
}

vector<LIST> initList(TOTAL &t)
{
	int i, listNum;
	double listVal;

	ifstream read("data.txt");

	read>>t.numArray;
	vector<LIST> myList(t.numArray);

	for(i = 0; i < t.numArray; i++)
		myList[i].avg = 0.0;

	while(read>>listNum>>listVal)
	{
		myList[listNum].data.push_back(listVal);
		myList[listNum].avg += listVal; // accumulate
	}

	for(i = 0; i < t.numArray; i++)
	{
		sort(myList[i].data.begin(),myList[i].data.end());

		myList[i].category = i;
		myList[i].listLen = myList[i].data.size();

		myList[i].avg /= myList[i].listLen;
		myList[i].median = listMedian(myList,i);

		myList[i].max = *max_element(myList[i].data.begin(),myList[i].data.end());
		myList[i].maxLen = count(myList[i].data.begin(),myList[i].data.end(),myList[i].max);

		myList[i].min = *min_element(myList[i].data.begin(),myList[i].data.end());
		myList[i].minLen = count(myList[i].data.begin(),myList[i].data.end(),myList[i].min);
	}

	return myList;
}

double listMedian(vector<LIST> list,int i)
{
	double p1, p2, ret;

	if(list[i].listLen == 0)
		ret = 0.0;
	else if(list[i].listLen % 2 == 0)
		ret = list[i].data[list[i].data.size()/2];
	else
	{
		p1 = list[i].data[list[i].data.size()/2];
		p2 = list[i].data[list[i].data.size()/2 + 1];
		
		ret = (p1 + p2)/2;
	}
	
	return ret;
}

void initTotal(TOTAL &t, vector<LIST> list)
{	
	int c1, c2, c3, c4;

	t.most = *max_element(list.begin(),list.end(),lenComp);
	t.least = *min_element(list.begin(),list.end(),lenComp);
	t.max = *max_element(list.begin(),list.end(),maxComp);
	t.min = *min_element(list.begin(),list.end(),minComp);

	t.arMost = findLen(t,list,c1,1);
	t.arLeast = findLen(t,list,c2,2);
	t.arMax = findLen(t,list,c3,3);
	t.arMin = findLen(t,list,c4,4);	

	t.mostLen = c1;
	t.leastLen = c2;
	t.maxLen = c3;
	t.minLen = c4;

	t.medMax = totalMedian(t,list,1);
	t.medMin = totalMedian(t,list,2);
	t.medAvg = totalMedian(t,list,3);

	return;
}

vector<int> findLen(TOTAL &t, vector<LIST> list, int &count, int choice)
{
	int i;
	bool comp;

	vector<int> tmp;
	tmp.resize(t.numArray);

	count = -1;
	for(i = 0; i < t.numArray; i++)
	{
		comp = 0;
		switch(choice)
		{
			case 1:
				if(list[i].listLen == t.most.listLen) comp = 1;
				break;
			case 2:
				if(list[i].listLen == t.least.listLen) comp = 1;
				break;
			case 3:
				if(list[i].max == t.max.max) comp = 1;
				break;
			case 4:
				if(list[i].min == t.min.min) comp = 1;
				break;
		}
		if(comp)
		{
			count++;
			tmp.insert(tmp.begin() + count,list[i].category);
		}
	}

	return tmp;
}

double totalMedian(TOTAL t, vector<LIST> list, int choice)
{
	int i, ind;
	double p1, p2, ret;

	ind = (int) (t.numArray/2.0);

	switch(choice)
	{
		case 1:
			sort(list.begin(),list.end(),maxComp);
			p1 = list[ind].max;
			p2 = list[ind+1].max;
			break;
		case 2:
			sort(list.begin(),list.end(),minComp);
			p1 = list[ind].min;
			p2 = list[ind+1].min;
			break;
		case 3:
			sort(list.begin(),list.end(),avgComp);
			p1 = list[ind].avg;
			p2 = list[ind+1].avg;
			break;
	}

	if(t.numArray % 2 == 0)
		ret = p1;
	else
		ret = (p1+p2)/2.0;

	return ret;
}

bool lenComp(LIST a, LIST b) { return a.listLen < b.listLen; }
bool maxComp(LIST a, LIST b) { return a.max < b.max; }
bool minComp(LIST a, LIST b) { return a.min < b.min; }
bool avgComp(LIST a, LIST b) { return a.avg < b.avg; }
bool medComp(LIST a, LIST b) { return a.median < b.median; }

void printOutput(TOTAL t, vector<LIST> list)
{
	int i, input;

	cout<<"======="<<endl<<"Output:"<<endl<<"======="<<endl<<endl;
		printMost(t,list);
	printLeast(t,list);
	printMax(t,list);
	printMin(t,list);

	// Avg, max, and min values for each list
	cout<<right<<setw(4)<<"List"<<setw(14)<<"Max"<<setw(14)<<"Min";
	cout<<setw(14)<<"Avg"<<setw(14)<<"Median"<<setw(14)<<"Length"<<endl;
	cout<<"=========================================================================="<<endl;
	for(i = 0; i < t.numArray; i++)
	{
		cout<<right
			<<setw(4)<<i
			<<setw(14)<<list[i].max
			<<setw(14)<<list[i].min
			<<setw(14)<<list[i].avg
			<<setw(14)<<list[i].median
			<<setw(14)<<list[i].listLen
			<<endl;
	}
	cout<<endl; 

	// Median user input
	cout<<"Enter category number (median):";
	cin>>input;
	cout<<endl;
	cout<<"The median value for list "<<input<<" is "<<list[input].median<<endl;
	cout<<endl;

	// Ascending user input
	cout<<"Enter category number (ascending):";
	cin>>input;
	cout<<endl;
	cout<<"List["<<input<<"]"<<endl;
	sort(list[input].data.begin(),list[input].data.end());
	for(i = 0; i < list[input].data.size(); i++)
	{
		cout<<list[input].data[i]<<endl;
	}
	cout<<endl;

	// Descending user input
	cout<<"Enter category number (descending):";
	cin>>input;
	cout<<endl;
	cout<<"List["<<input<<"]"<<endl;
	sort(list[input].data.begin(),list[input].data.end());
	reverse(list[input].data.begin(),list[input].data.end());	
	for(i = 0; i < list[input].data.size(); i++)
	{
		cout<<list[input].data[i]<<endl;
	}
	cout<<endl;

	cout<<"The median of max values is "<<t.medMax<<endl<<endl;
	cout<<"The median of min values is "<<t.medMin<<endl<<endl;
	cout<<"The median of avg values is "<<t.medAvg<<endl<<endl;

	cout<<"Category ascending based on avg:"<<endl<<endl;
	cout<<"List\tAvg"<<endl;
	cout<<"=============="<<endl;
	sort(list.begin(),list.end(),avgComp);
	for(i = 0; i < t.numArray; i++)
		cout<<list[i].category<<"\t"<<list[i].avg<<endl;
	cout<<endl;

	cout<<"Category descending based on med:"<<endl<<endl;
	cout<<"List\tMedian"<<endl;
	cout<<"=============="<<endl;
	sort(list.begin(),list.end(),medComp);
	reverse(list.begin(),list.end());
	for(i = 0; i < t.numArray; i++)
		cout<<list[i].category<<"\t"<<list[i].median<<endl;
	cout<<endl;

	return;
}

/*
void printMost(TOTAL t, vector<LIST> list, int choice)
{
	int i, len, val;

	vector<int> arTmp(t.numArray);

	switch(choice)
	{
		case 1:
			len = t.mostLen;
			copy(t.arMost.begin(),t.arMost.end(),arTmp.begin());
			val = t.most.listLen;
			break;
		case 2:
			len = t.leastLen;
			break;
		case 3:
			len = t.maxLen;
			break;
		case 4:
			len = t.minLen;
			break;
	}

	cout<<"List";
	if(len > 0)
	{
		cout<<"s ";
		for(i = 0; i <= len; i++)
		{
			cout<<arTmp[i];
			if(i != len) cout<<",";
		}
	}
	else
		cout<<" "<<arTmp[0];
	cout<<" had the most values with "<<val<<"."<<endl;
	cout<<endl;

	return;
} */

void printMost(TOTAL t, vector<LIST> list)
{
	int i;

	cout<<"List";
	if(t.mostLen > 0)
	{
		cout<<"s ";
		for(i = 0; i <= t.mostLen; i++)
		{
			cout<<t.arMost[i];
			if(i != t.mostLen) cout<<",";
		}
	}
	else
		cout<<" "<<t.arMost[0];
	cout<<" had the most values with "<<t.most.listLen<<"."<<endl;
	cout<<endl;

	return;
}

void printLeast(TOTAL t, vector<LIST> list)
{
	int i;

	cout<<"List";
	if(t.leastLen > 0)
	{
		cout<<"s ";
		for(i = 0; i <= t.leastLen; i++)
		{
			cout<<t.arLeast[i];
			if(i != t.leastLen) cout<<",";
		}
	}
	else
		cout<<" "<<t.arLeast[0];
	cout<<" had the least values with "<<t.least.listLen<<"."<<endl;
	cout<<endl;

	return;
}

void printMax(TOTAL t, vector<LIST> list)
{
	int i;

	cout<<"List";
	if(t.maxLen > 0)
	{
		cout<<"s ";
		for(i = 0; i <= t.maxLen; i++)
		{
			cout<<t.arMax[i];
			if(i != t.maxLen) cout<<",";
		}
	}
	else
		cout<<" "<<t.arMax[0];
	cout<<" had the highest value, which was "<<t.max.max<<"."<<endl;
	for(i = 0; i <= t.maxLen; i++)
		cout<<"This value appears "<<list[t.arMax[i]].maxLen<<" times in list "<<t.arMax[i]<<"."<<endl;
	cout<<endl;

	return;
}

void printMin(TOTAL t, vector<LIST> list)
{
	int i;

	cout<<"List";
	if(t.minLen > 0)
	{
		cout<<"s ";
		for(i = 0; i <= t.minLen; i++)
		{
			cout<<t.arMin[i];
			if(i != t.minLen) cout<<",";
		}
	}
	else
		cout<<" "<<t.arMin[0];
	cout<<" had the lowest value, which was "<<t.min.min<<"."<<endl;
	for(i = 0; i <= t.minLen; i++)
		cout<<"This value appears "<<list[t.arMin[i]].minLen<<" times in list "<<t.arMin[i]<<"."<<endl;
	cout<<endl;

	return;
}
