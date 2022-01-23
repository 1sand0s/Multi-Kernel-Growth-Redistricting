#include "County.h"
#include <stdio.h>
#include <algorithm>

using namespace std;
using namespace rapidjson;

County::County(rapidjson::Value& val)
{
	parseJsonVal(val);			
}

void County::parseJsonVal(Value& val)
{

	Value& a=val["fields"].GetObject();
	
	name = a["name"].GetString();

	transform(name.begin(), name.end(), name.begin(), ::tolower);

	name+=" county";

	Value& b=(((a["geo_shape"].GetObject())["coordinates"].GetArray())[0]).GetArray();
	

	for(SizeType i=0; i<b.Size(); i++)
	{
		boundary.push_back(std::pair<double,double>((b[i])[0].GetDouble()*1e6, -(b[i])[1].GetDouble()*1e6));
	}	

	//area = a["aland"].GetInt();
	
	centerx = ((val["geometry"].GetObject())["coordinates"].GetArray())[0].GetDouble()*1e6;
	centery	= ((val["geometry"].GetObject())["coordinates"].GetArray())[1].GetDouble()*1e6;

	computeBoundingBox();
}

County::~County()
{

}


void County::computeBoundingBox()
{
	s_x = s_y = 1e9;
	l_x = l_y = -1e9;

	for(int i=0; i<boundary.size(); i++)
        {
               	s_x = s_x > boundary.at(i).first ? boundary.at(i).first : s_x;
		s_y = s_y > boundary.at(i).second? boundary.at(i).second : s_y;

		l_x = l_x < boundary.at(i).first ? boundary.at(i).first : l_x;
                l_y = l_y < boundary.at(i).second? boundary.at(i).second : l_y;
        }
}

void County::scaleBoundary(double xMin, double yMin, double xRange, double yRange, int xScale, int yScale)
{
	for(int i=0;i<boundary.size(); i++)
	{
		boundary.at(i).first  = ((boundary.at(i).first - xMin)/(xRange))*xScale;
		boundary.at(i).second = ((boundary.at(i).second - yMin)/(yRange))*yScale;
	}

	centerx = ((centerx - xMin)/xRange)*xScale;
	centery = ((centery - yMin)/yRange)*yScale;
}

void County::printBoundary(WINDOW*& win)
{
	for(int i=0;i<boundary.size();i++)
	{
		mvaddstr(boundary.at(i).second,boundary.at(i).first, "*");
		//mvwaddch(win, boundary.at(i).second, boundary.at(i).first, '*');
	}
}

const string  County::getName()
{
	return name;
}

void County::setCharacteristics(double val, int ind)
{
	if(Characteristics.size() > ind)
		Characteristics.at(ind) = val;
	else
		Characteristics.push_back(val);
}

double County::getCharacteristics(int ind)
{
	if(Characteristics.size() > ind)
		return Characteristics.at(ind);
	else
		return 0;
}

void County::setCharacteristicVariances(double val, int ind)
{
        if(CharacteristicVariances.size() > ind)
                CharacteristicVariances.at(ind) = val;
        else
                CharacteristicVariances.push_back(val);

}

double County::getCharacteristicVariances(int ind)
{
	if(CharacteristicVariances.size() > ind)
        	return CharacteristicVariances.at(ind);
	else
		return 0;
}

void County::setBackgroundColor(int val, int ind)
{
	if(bg.size() > ind)
                bg.at(ind) = val;
        else
                bg.push_back(val);

}

int County::getBackgroundColor(int i)
{
	return bg.at(i);
}

void County::addNeighbor(County*& county)
{
	neighbors.push_back(county);
}

bool County::isaNeighbor(County*& county)
{
	bool noOverlap = s_x > county->l_x ||
                     	 county->s_x > l_x ||
                     	 s_y > county->l_y ||
                     	 county->s_y > l_y;

	return !noOverlap;
}

void County::updatePartyLeanings(double max)
{
	partyAVotes = (getCharacteristics(EDUCATION_MAP)/max)*getCharacteristics(POPULATION_MAP)*100;

	partyBVotes = getCharacteristics(POPULATION_MAP) - partyAVotes;

	setCharacteristics(partyAVotes > partyBVotes ? 1 : 0, POLITICAL_MAP);

	//Party A favors Education	
}

void County::removePointsinBox(double s_x, double s_y, double l_x, double l_y)
{
	for(int j=0; j<boundary.size(); j++)
	{
		double px = boundary.at(j).first;
		double py = boundary.at(j).second;

		if(px>=s_x && px<=l_x && py>=s_y && py<=l_y)
		{
			boundary.at(j).first=1000;
			boundary.at(j).second=1000;
		}
	}	
}
