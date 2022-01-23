#include "../rapidjson-master/include/rapidjson/document.h"
#include <vector>
#include <ncurses.h>
#include "GlobalDefs.h"

class County
{
	public:
		County(rapidjson::Value&);
		~County();

		struct CountyCompare 
		{
			int ind;
			
			CountyCompare(int ind2)
			{
				ind=ind2;
			}

      			bool operator()(County* l, County* r) 
			{
        			return (l->getCharacteristics(ind) < r->getCharacteristics(ind));
      			}
    		};

		void parseJsonVal(rapidjson::Value&);		
		void computeBoundingBox();
		void scaleBoundary(double, double, double, double, int, int);
		void printBoundary(WINDOW*&);
		const std::string getName();
		void setCharacteristics(double, int);
		double getCharacteristics(int);
		void setCharacteristicVariances(double, int);
		double getCharacteristicVariances(int);
		void setBackgroundColor(int,int);
		int getBackgroundColor(int);
		void addNeighbor(County*&);
		bool isaNeighbor(County*&);
		void updatePartyLeanings(double);
		void removePointsinBox(double,double,double,double);
				
		double s_x,s_y,l_x,l_y;
		double area;
		double centerx;
		double centery;
		std::vector<County*> neighbors;
		int partyAVotes;
		int partyBVotes;
	
	private:		
		std::vector<std::pair<double, double> > boundary;
		std::vector<int> bg;
		std::vector<double> Characteristics;
		std::vector<double> CharacteristicVariances;	
		std::string name; 
};
