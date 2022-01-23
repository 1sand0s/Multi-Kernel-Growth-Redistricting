#include <vector>
#include "County.h"

class District
{
	public:
		District(County*&);
		~District();
		
		double calculateDistance(County*&);
		void addCounty(County*&);
		void cleanupBoundaries();		

		double total_pop;
		std::vector<County*> counties;

		int partyAVotes;
		int partyBVotes;
};

