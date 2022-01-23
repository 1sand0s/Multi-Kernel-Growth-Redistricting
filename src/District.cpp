#include "District.h"
#include <cmath>

using namespace std;

District::District(County*& county)
{
	counties.push_back(county);
	total_pop = county->getCharacteristics(POPULATION_MAP);
	partyAVotes = county->partyAVotes;
	partyBVotes = county->partyBVotes;
}

District::~District()
{
}

double District::calculateDistance(County*& county)
{
	return hypot(counties.at(0)->centerx-county->centerx, counties.at(0)->centery-county->centery);
}

void District::addCounty(County*& county)
{
	counties.push_back(county);
	total_pop+=county->getCharacteristics(POPULATION_MAP);
	partyAVotes+=county->partyAVotes;
	partyBVotes+=county->partyBVotes;
}

void District::cleanupBoundaries()
{
	for(int i=0; i<counties.size(); i++)
		counties.at(i)->computeBoundingBox();

	for(int i=0; i<counties.size()-1; i++)
        {
		for(int j=i+1; j<counties.size(); j++)
		{
                	counties.at(i)->removePointsinBox(counties.at(j)->s_x, counties.at(j)->s_y, counties.at(j)->l_x, counties.at(j)->l_y);
                	counties.at(j)->removePointsinBox(counties.at(i)->s_x, counties.at(i)->s_y, counties.at(i)->l_x, counties.at(i)->l_y);
        	}
	}	
}
