#include "District.h"
#include <vector>
#include <fstream>
#include <ncurses.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cmath>
#include <time.h>

#define NCURSES_ON
//#define DEBUG_DISTRICTING_PRINT  

using namespace std;
using namespace rapidjson;

void readJsonFile(char*, Document&);
void readCSVFile(char*, vector<pair<string, int> >&);
void makeCounties(char*, vector<County*>&, int, int, double&, vector<pair<string, int> >&);
void makePoliticalLeanings(char*, vector<County*>&);
void updateVariances(double, County*&, int);
void displayTopic(WINDOW*&, string, vector<County*>&, int);
int colornum(int,int);
short curs_color(int);
void init_colorpairs();
void generateCenters(vector<int>&, int, vector<County*>&, int);
void computeNeighbors(vector<County*>&);
void makeCongressionalDistricts(vector<District*>&, vector<County*>&, vector<int>&, double);
string int2String(int val)
{
	stringstream ss;
    	ss << val;
    	string str;
    	ss >> str;

	return str;
}

int main(int argc, char** argv)
{
	vector<County*> counties;
	vector<pair<string, int> > population;	
	vector<int> centers;	
	vector<District*> districts;

	double total_pop=0;

#ifdef NCURSES_ON
	initscr();

        keypad(stdscr, TRUE);
        int h, w;
        getmaxyx(stdscr, h, w);

#else
	int h,w;h=w=100;
#endif

	readCSVFile(argv[2], population);
	makeCounties(argv[1], counties, h, w, total_pop, population);
	makePoliticalLeanings(argv[3], counties);
	generateCenters(centers, atoi(argv[4]), counties, atoi(argv[5]));
	computeNeighbors(counties);
	makeCongressionalDistricts(districts, counties, centers, total_pop/centers.size());
	
#ifdef NCURSES_ON
	printw("Reference"); 
    	refresh(); 		
	WINDOW * win = newwin(h, w, 1, 1);

	start_color();
    	init_colorpairs();	
	
	displayTopic(win, "Population Map", counties, POPULATION_MAP);
	displayTopic(win, "Universities Map", counties, EDUCATION_MAP);

	std::string countyVoteShare = "County Political Alignment Map\n\nParty Blue\t\tParty Red\n";
	int partyAdist = 0;

	for(int j=0; j<counties.size(); j++)
	{
		partyAdist+=counties.at(j)->partyAVotes > counties.at(j)->partyBVotes ? 1 : 0;
	}	
	countyVoteShare+=int2String(partyAdist)+"\t\t\t"+int2String(counties.size()-partyAdist);
	
	displayTopic(win, countyVoteShare, counties, POLITICAL_MAP);
	displayTopic(win, "Multi-Kernel Seed Center Map For Redistricting", counties, CENTER_MAP);
	
	std::string voteShare = "Congressional District Map\n\nParty Blue\t\tParty Red\n";
	partyAdist=0;
	

	for(int j=0; j<districts.size(); j++)	
	{
		districts.at(j)->cleanupBoundaries();
		for(int i=0; i<districts.at(j)->counties.size(); i++)
		{
			districts.at(j)->counties.at(i)->setBackgroundColor(districts.at(j)->partyAVotes > districts.at(j)->partyBVotes ? 1 : 4, HIGHLIGHT_MAP);
			districts.at(j)->counties.at(i)->setCharacteristics(districts.at(j)->partyAVotes > districts.at(j)->partyBVotes ? 1 : 4, HIGHLIGHT_MAP);
		}		
		partyAdist+=districts.at(j)->partyAVotes > districts.at(j)->partyBVotes ? 1 : 0;
	}
	voteShare+=int2String(partyAdist)+"\t\t\t"+int2String(atoi(argv[4])-partyAdist);
	displayTopic(win, voteShare, counties, HIGHLIGHT_MAP);

    	delwin(win);
    	endwin();
#endif
	return 0;
}

void readJsonFile(char* fileName, Document& doc)
{
        string line;
        string content;
        ifstream fileIn;

        fileIn.open(fileName);
        if (fileIn.is_open())
        {
                while (getline(fileIn, line))
                {
                        content += line;
                }
                fileIn.close();
        }
	
	doc.Parse(content.c_str());
}

void readCSVFile(char* fileName, vector<pair<string, int> >& vec)
{
        string line;
        string content;
        ifstream fileIn;

        fileIn.open(fileName);
        if (fileIn.is_open())
        {
                while (getline(fileIn, line, ','))
                {
			transform(line.begin(), line.end(), line.begin(), ::tolower);

                        string line2;
				
			getline(fileIn, line2);

			vec.push_back(pair<string, int>(line,atoi(line2.c_str())));
                }
                fileIn.close();
        }
}

int colornum(int fg, int bg)
{
    int B, bbb, ffff;

    B = 1 << 7;
    bbb = (7 & bg) << 4;
    ffff = 7 & fg;

    return (B | bbb | ffff);
}

short curs_color(int fg)
{
    switch (7 & fg) {           /* RGB */
    case 0:                     /* 000 */
        return (COLOR_BLACK);
    case 1:                     /* 001 */
        return (COLOR_BLUE);
    case 2:                     /* 010 */
        return (COLOR_GREEN);
    case 3:                     /* 011 */
        return (COLOR_CYAN);
    case 4:                     /* 100 */
        return (COLOR_RED);
    case 5:                     /* 101 */
        return (COLOR_MAGENTA);
    case 6:                     /* 110 */
        return (COLOR_YELLOW);
    case 7:                     /* 111 */
        return (COLOR_WHITE);
    }
}

void init_colorpairs()
{
    int fg, bg;
    int colorpair;

    for (bg = 0; bg <= 7; bg++) {
        for (fg = 0; fg <= 7; fg++) {
            colorpair = colornum(fg, bg);
            init_pair(colorpair, curs_color(fg), curs_color(bg));
        }
    }
}

void makeCounties(char* fileName, vector<County*>& counties, int h, int w, double& total_pop, vector<pair<string, int> >& population)
{
	
	Document doc;
	double s_x,s_y,l_x,l_y;
	s_x=s_y=1e9;
	l_x=l_y=-1e9;

	readJsonFile(fileName, doc);

	Value a(kArrayType);

        a = doc.GetArray();

        for(SizeType i=0; i<a.Size(); i++)
        {
                County* county = new County(a[i]);

                for(int j=0; j<population.size(); j++)
                {

                        if(county->getName() == population.at(j).first)
                        {
                                county->setCharacteristics(population.at(j).second, POPULATION_MAP);
                                population.erase(population.begin()+j);
                                break;
                        }
                }

                s_x = s_x > county->s_x ? county->s_x : s_x;
                s_y = s_y > county->s_y ? county->s_y : s_y;

                l_x = l_x < county->l_x ? county->l_x : l_x;
                l_y = l_y < county->l_y ? county->l_y : l_y;

                counties.push_back(county);
	}

	for(int i=0; i<counties.size(); i++)
        {
                counties.at(i)->scaleBoundary(s_x,s_y,l_x-s_x,l_y-s_y,w-10,h-5);
		
                total_pop += counties.at(i)->getCharacteristics(POPULATION_MAP);
        }


	for(int i=0; i<counties.size(); i++)
        {
                updateVariances(total_pop/counties.size(), counties.at(i), POPULATION_MAP);
	}

}

void makePoliticalLeanings(char* fileName, vector<County*>& counties)
{
	Document doc;

	readJsonFile(fileName, doc);

	Value a(kArrayType);

        a = doc.GetArray();

	for(int i=0;i<counties.size();i++)
	{
		counties.at(i)->setCharacteristics(0, POLITICAL_MAP);
	}

        for(SizeType i=0; i<a.Size(); i++)
	{
		string name = ((a[i])["fields"].GetObject())["county"].GetString();
		transform(name.begin(), name.end(), name.begin(), ::tolower);

		name+=" county";
		
		for(int j=0; j<counties.size(); j++)
		{
			if(name == counties.at(j)->getName())
			{
				counties.at(j)->setCharacteristics(counties.at(j)->getCharacteristics(EDUCATION_MAP)+1, EDUCATION_MAP);
			}
		}
	}


	double meanCollegeCount = (a.Size()*1.0)/counties.size();

	for(int i=0; i<counties.size(); i++)
	{
		updateVariances(meanCollegeCount, counties.at(i), EDUCATION_MAP);
		counties.at(i)->updatePartyLeanings(a.Size()*1.0);
		counties.at(i)->setBackgroundColor(counties.at(i)->getCharacteristics(POLITICAL_MAP) ? 1 : 4, POLITICAL_MAP);
	}
}

void updateVariances(double mean, County*& county, int ind)
{
	double variance_percent = ((county->getCharacteristics(ind) - mean)/mean)*100.0;
	county->setCharacteristicVariances(variance_percent, ind);

        if(variance_percent > 80)
		county->setBackgroundColor(4,ind);

	else if(variance_percent > 60 && variance_percent <=80)
		county->setBackgroundColor(6,ind);

	else if(variance_percent > 40 && variance_percent <=60)
		county->setBackgroundColor(2,ind);

	else if(variance_percent > 30 && variance_percent <=40)
		county->setBackgroundColor(3,ind);

	else
		county->setBackgroundColor(1,ind);

}

void displayTopic(WINDOW*& win, string topic, vector<County*>& counties, int ind)
{
	clear();
        printw(topic.c_str());

        refresh();

        sort(counties.begin(),counties.end(),County::CountyCompare(ind));

        for(int i=0; i<counties.size(); i++)
        {
                attron(COLOR_PAIR(colornum(7, counties.at(i)->getBackgroundColor(ind))));
                counties.at(i)->printBoundary(win);
                attroff(COLOR_PAIR(colornum(7, counties.at(i)->getBackgroundColor(ind))));
        }
	wrefresh(win);
        getch();
}

void generateCenters(vector<int>& centers, int center_size, vector<County*>& counties, int GerryManderingOn)
{
	
	/*int j=0;

	if(GerryManderingOn == 1)
	{
		for(; j<counties.size() && j<(center_size/2 + 5); j++)
		{
			if(counties.at(j)->partyAVotes > counties.at(j)->partyBVotes)
			{
				centers.push_back(j);
			}
		}
	}*/

	srand(time(NULL));
	for(int j=0; j<center_size; j++)
	{
		int r;
		//srand(time(NULL));
		do
		{
			r = rand()%counties.size();
		
		}while(find(centers.begin(), centers.end(), r)!=centers.end());

		centers.push_back(r);
	}

	for(int i=0; i<counties.size(); i++)
	{
		counties.at(i)->setCharacteristics(0, CENTER_MAP);
		counties.at(i)->setCharacteristics(-1, CONGRESS_MAP);
		counties.at(i)->setBackgroundColor(1, CENTER_MAP);
		counties.at(i)->setBackgroundColor(1, CONGRESS_MAP);
	}
	

	for(int i=0; i<centers.size(); i++)
	{
		//printf("%d\n",centers.at(i));
		counties.at(centers.at(i))->setCharacteristics(1, CENTER_MAP);
		counties.at(centers.at(i))->setCharacteristics(i, CONGRESS_MAP);
		counties.at(centers.at(i))->setBackgroundColor(4, CENTER_MAP);
		printf("%s %f\n",counties.at(centers.at(i))->getName().c_str(),counties.at(centers.at(i))->getCharacteristics(POLITICAL_MAP));
	}
}

void computeNeighbors(vector<County*>& counties)
{
	for(int i=0; i<counties.size()-1; i++)
	{
		for(int j=i+1; j<counties.size(); j++)
		{
			if(counties.at(i)->isaNeighbor(counties.at(j)))
			{
				counties.at(i)->addNeighbor(counties.at(j));
				counties.at(j)->addNeighbor(counties.at(i));
			}
		}
	}
}

void makeCongressionalDistricts(vector<District*>& districts, vector<County*>& counties, vector<int>& centers, double max_pop)
{
	
	for(int i=0; i<centers.size(); i++)
	{
		District* district = new District(counties.at(centers.at(i)));

		districts.push_back(district);
	}


	int left_dis = 0;
	int prev_left_dis = -1;
	
	while(left_dis < (counties.size()-1))
	{

		for(int i=0; i<districts.size(); i++)
		{
			vector<int> ind1;
			vector<int> ind2;
			vector<double> dist;

			if(districts.at(i)->total_pop < max_pop || (left_dis == prev_left_dis))
			{
				for(int j=0; j<districts.at(i)->counties.size(); j++)
				{
					for(int k=0; k<districts.at(i)->counties.at(j)->neighbors.size(); k++)
					{
						if(abs(districts.at(i)->counties.at(j)->neighbors.at(k)->getCharacteristics(CONGRESS_MAP) + 1.0) < 1e-3)
						{	
							if(districts.at(i)->total_pop + districts.at(i)->counties.at(j)->neighbors.at(k)->getCharacteristics(POPULATION_MAP) < max_pop || (left_dis == prev_left_dis))
							{
								ind1.push_back(j);
								ind2.push_back(k);
								dist.push_back(districts.at(i)->calculateDistance(districts.at(i)->counties.at(j)->neighbors.at(k)));
							}
						}
					}				
				}
			}
			
			if(dist.size() > 0)
			{
				int min = (min_element(dist.begin(), dist.end()) - dist.begin());

				districts.at(i)->addCounty(districts.at(i)->counties.at(ind1.at(min))->neighbors.at(ind2.at(min)));
				districts.at(i)->counties.at(ind1.at(min))->neighbors.at(ind2.at(min))->setCharacteristics(i, CONGRESS_MAP);
			}
		}


#ifdef DEBUG_DISTRICTING_PRINT
		
		printf("\n\n\n\n");
		for(int i=0;i<districts.size();i++)
		{
			printf("\n\nCenter : %s %f %f\n", districts.at(i)->counties.at(0)->getName().c_str(), districts.at(i)->total_pop, max_pop);

			for(int j=1; j<districts.at(i)->counties.size(); j++)
			{
				printf("%s\n",districts.at(i)->counties.at(j)->getName().c_str());
			}
		}
#endif
		prev_left_dis = left_dis;
                left_dis = 0;
		
                for(int i=0; i<counties.size(); i++)
                {
                        left_dis = (counties.at(i)->getCharacteristics(CONGRESS_MAP)!=-1) ? (left_dis + 1) : left_dis;
                }

#ifdef DEBUG_DISTRICTING_PRINT
		printf("\n%d %d %d", left_dis, prev_left_dis, counties.size());
		
		cin.get();
#endif
	}
}
