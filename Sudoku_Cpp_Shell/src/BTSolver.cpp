#include"BTSolver.hpp"

using namespace std;

// =====================================================================
// Constructors
// =====================================================================

BTSolver::BTSolver ( SudokuBoard input, Trail* _trail,  string val_sh, string var_sh, string cc )
: sudokuGrid( input.get_p(), input.get_q(), input.get_board() ), network( input )
{
	valHeuristics = val_sh;
	varHeuristics = var_sh; 
	cChecks =  cc;

	trail = _trail;
}

// =====================================================================
// Consistency Checks
// =====================================================================

// Basic consistency check, no propagation done
bool BTSolver::assignmentsCheck ( void )
{
	for ( Constraint c : network.getConstraints() )
		if ( ! c.isConsistent() )
			return false;

	return true;
}

// =================================================================
// Arc Consistency
// =================================================================
bool BTSolver::arcConsistency ( void )
{
    vector<Variable*> toAssign;
    vector<Constraint*> RMC = network.getModifiedConstraints();
    for (Constraint * c : RMC)
    {
        vector<Variable*> LV = c->vars;
        for (Variable * v : LV)
        {
            if(v->isAssigned())
            {
                vector<Variable*> Neighbors = network.getNeighborsOfVariable(v);
                int assignedValue = v->getAssignment();
                for (Variable * n : Neighbors)
                {
                    Domain D = n->getDomain();
                    if(D.contains(assignedValue))
                    {
                        if (D.size() == 1)
                            return false;
                        if (D.size() == 2)
                            toAssign.push_back(n);
                        trail->push(n);
                        n->removeValueFromDomain(assignedValue);
                    }
                }
            }
        }
    }
    if (!toAssign.empty())
    {
        for (Variable * a : toAssign)
        {
            Domain D = a->getDomain();
            vector<int> assign = D.getValues();
            trail->push(a);
            a->assignValue(assign[0]);
        }
        return arcConsistency();
    }
    return network.isConsistent();
}

/**
 * Part 1 TODO: Implement the Forward Checking Heuristic
 *
 * This function will do both Constraint Propagation and check
 * the consistency of the network
 *
 * (1) If a variable is assigned then eliminate that value from
 *     the square's neighbors.
 *
 * Note: remember to trail.push variables before you change their domain
 * Return: a pair of a map and a bool. The map contains the pointers to all MODIFIED variables, mapped to their MODIFIED domain. 
 * 		   The bool is true if assignment is consistent, false otherwise.
 */
pair<map<Variable*,Domain>,bool> BTSolver::forwardChecking ( void )
{
	map<Variable*, Domain> modded;
	vector<Constraint*> RMC = network.getModifiedConstraints();
	for (Constraint * c : RMC)
	{
		vector<Variable*> LV = c->vars;
		for (Variable * v : LV)
		{
			if (v->isAssigned())
			{
				vector<Variable*> Neighbors = network.getNeighborsOfVariable(v);
				int assignedValue = v->getAssignment();
				for (Variable* n : Neighbors)
				{
					Domain D = n->getDomain();
					if (D.contains(assignedValue))
					{
						if (D.size() == 1)
							return make_pair(modded, false);
						trail->push(n);
						n->removeValueFromDomain(assignedValue);
						modded[n] = n->getDomain();
					}
				}
			}
		}
	}
	return make_pair(modded, true);
}

/**
 * Part 2 TODO: Implement both of Norvig's Heuristics
 *
 * This function will do both Constraint Propagation and check
 * the consistency of the network
 *
 * (1) If a variable is assigned then eliminate that value from
 *     the square's neighbors.
 *
 * (2) If a constraint has only one possible place for a value
 *     then put the value there.
 *
 * Note: remember to trail.push variables before you change their domain
 * Return: a pair of a map and a bool. The map contains the pointers to all variables that were assigned during 
 *         the whole NorvigCheck propagation, and mapped to the values that they were assigned. 
 *         The bool is true if assignment is consistent, false otherwise.
 */
pair<map<Variable*,int>,bool> BTSolver::norvigCheck ( void )
{
	pair<map<Variable*, Domain>, bool> result;
	result = forwardChecking();

	if (result.second == false)
		return make_pair(map<Variable*, int>(), false);

	map<Variable*, int> assigned;
	map<int, int> domainCounter;
	vector<int> values;
	int target;
	for (Variable* v : network.getVariables())
	{
		if (!(v->isAssigned()))
		{
			Domain D = v->getDomain();
			values = D.getValues();
			for (int i = 0; i < values.size(); ++i)
			{
				if (domainCounter.find(values[i]) == domainCounter.end())
				{
					domainCounter[values[i]] = 1;
				}
				else
				{
					++(domainCounter[values[i]]);
				}
			}
		}
	}
	for (map<int, int>::iterator i = domainCounter.begin(); i != domainCounter.end(); ++i)
	//for (auto i : domainCounter)
	{
		if (i->second == 1)
		{
			target = i->first;
			for (Variable* v : network.getVariables())
			{
				if (!(v->isAssigned()))
				{
					Domain D = v->getDomain();
					if (D.contains(target))
					{
						v->assignValue(target);
						assigned[v] = target;
						break;
					}
				}
			}
		}
	}


    return make_pair(assigned, true);
	//return make_pair(map<Variable*, int>(), false);
}

/**
 * Optional TODO: Implement your own advanced Constraint Propagation
 *
 * Completing the three tourn heuristic will automatically enter
 * your program into a tournament.
 */
bool BTSolver::getTournCC ( void )
{
	return false;
}

// =====================================================================
// Variable Selectors
// =====================================================================

// Basic variable selector, returns first unassigned variable
Variable* BTSolver::getfirstUnassignedVariable ( void )
{
	for ( Variable* v : network.getVariables() )
		if ( !(v->isAssigned()) )
			return v;

	// Everything is assigned
	return nullptr;
}

/**
 * Part 1 TODO: Implement the Minimum Remaining Value Heuristic
 *
 * Return: The unassigned variable with the smallest domain
 */
Variable* BTSolver::getMRV(void)
{
	Variable *min = nullptr;
	int val = 2147483647; //INT_MAX

	for (Variable* v : network.getVariables())
	{
		if (!(v->isAssigned()))
		{
			Domain D = v->getDomain();
			if (D.size() < val)
			{
				val = D.size();
				min = v;
			}
		}
	}
	return min;
}

/**
 * Part 2 TODO: Implement the Minimum Remaining Value Heuristic
 *                with Degree Heuristic as a Tie Breaker
 *
 * Return: The unassigned variable with the smallest domain and affecting the most unassigned neighbors.
 * 		   If there are multiple variables that have the same smallest domain with the same number 
 * 		   of unassigned neighbors, add them to the vector of Variables.
 *         If there is only one variable, return the vector of size 1 containing that variable.
 */
vector<Variable*> BTSolver::MRVwithTieBreaker ( void )
{
	vector<Variable*> result;
	vector<Variable*> answer;
	vector<int> neighborCount;
	int countNeighbors = 0;
	int val = 2147483647; //INT_MAX
	//map<Variable*, int> degreeCount;
	int maxNeighbors = -1;

	for (Variable* v : network.getVariables())
	{
		if (!(v->isAssigned()))
		{
			Domain D = v->getDomain();
			if (D.size() < val)
			{
				val = D.size();
			}
		}
	}
	
	for (Variable* v : network.getVariables())
	{
		if(!(v->isAssigned()))
		{
			Domain D = v->getDomain();
			if( D.size() == val)
			{
				result.push_back(v);
			}
		}
	}
/**
	for (Variable* v : result)
	{
		vector<Variable*> neighbors = network.getNeighborsOfVariable(v);
		countNeighbors = 0;
		for(Variable* w : neighbors)
		{
			if(!(w->isAssigned()))
				++countNeighbors;
		}
		//degreeCount[v] = countNeighbors;
		//degreeCount.insert ( std::pair<Variable*,int>(v,countNeighbors) )
		if(countNeighbors > maxNeighbors)
			maxNeighbors = countNeighbors;
	}

	for (map<Variable*, int>::iterator m = degreeCount.begin(); m != degreeCount.end(); ++m)
	{
		if(m->second == maxNeighbors)
			answer.push_back(m->first);
	}
**/
	for (Variable* v : result)
	{
		vector<Variable*> neighbors = network.getNeighborsOfVariable(v);
		countNeighbors = 0;
		for(Variable* w : neighbors)
		{
			if(!(w->isAssigned()))
				++countNeighbors;
		}
		neighborCount.push_back(countNeighbors);
		if(countNeighbors > maxNeighbors)
			maxNeighbors = countNeighbors;
	}
	
	for (int i = 0; i < result.size(); ++i)
	{
		if(neighborCount[i] == maxNeighbors)
		{
			answer.push_back(result[i]);
		}
	}
	
	
	
	return answer;
}

/**
 * Optional TODO: Implement your own advanced Variable Heuristic
 *
 * Completing the three tourn heuristic will automatically enter
 * your program into a tournament.
 */
Variable* BTSolver::getTournVar ( void )
{
	return nullptr;
}

// =====================================================================
// Value Selectors
// =====================================================================

// Default Value Ordering
vector<int> BTSolver::getValuesInOrder ( Variable* v )
{
	vector<int> values = v->getDomain().getValues();
	sort( values.begin(), values.end() );
	return values;
}

/**
 * Part 1 TODO: Implement the Least Constraining Value Heuristic
 *
 * The Least constraining value is the one that will knock the least
 * values out of it's neighbors domain.
 *
 * Return: A list of v's domain sorted by the LCV heuristic
 *         The LCV is first and the MCV is last
 */
vector<int> BTSolver::getValuesLCVOrder ( Variable* v )
{	
	Domain mainD = v->getDomain();
	vector<Variable*> Neighbors = network.getNeighborsOfVariable(v);

	map<int, int> counter;
	vector<int> values = mainD.getValues();
	for (int i = 0; i < values.size(); ++i)
	{
		counter[values[i]] = 0;
	}

	vector<int> tempVals;

	for (int j = 0; j < Neighbors.size(); ++j)
	{
		Domain tempDomain = Neighbors[j]->getDomain();
		tempVals = tempDomain.getValues();
		for (int k = 0; k < values.size(); ++k)
		{
			for (int l = 0; l < tempVals.size(); ++l)
			{
				if (values[k] == tempVals[l])
				{
					++(counter[values[k]]);
					break;
				}
			}
		}
	}

	vector<int> sortedDomain;
	int index;
	int max = 2147483647;
	for (int n = 0; n < values.size(); ++n)
	{
		for (map<int, int>::iterator m = counter.begin(); m != counter.end(); ++m)
		{
			if (max > m->second)
			{
				index = m->first;
				max = m->second;
			}
		}
		sortedDomain.push_back(index);
		counter[index] = 2147483647;
		max = 2147483647;
	}

	return sortedDomain;
}

/**
 * Optional TODO: Implement your own advanced Value Heuristic
 *
 * Completing the three tourn heuristic will automatically enter
 * your program into a tournament.
 */
vector<int> BTSolver::getTournVal ( Variable* v )
{
	return vector<int>();
}

// =====================================================================
// Engine Functions
// =====================================================================

int BTSolver::solve ( float time_left)
{
	if (time_left <= 60.0)
		return -1;
	double elapsed_time = 0.0;
    clock_t begin_clock = clock();

	if ( hasSolution )
		return 0;

	// Variable Selection
	Variable* v = selectNextVariable();

	if ( v == nullptr )
	{
		for ( Variable* var : network.getVariables() )
		{
			// If all variables haven't been assigned
			if ( ! ( var->isAssigned() ) )
			{
				return 0;
			}
		}

		// Success
		hasSolution = true;
		return 0;
	}

	// Attempt to assign a value
	for ( int i : getNextValues( v ) )
	{
		// Store place in trail and push variable's state on trail
		trail->placeTrailMarker();
		trail->push( v );

		// Assign the value
		v->assignValue( i );

		// Propagate constraints, check consistency, recurse
		if ( checkConsistency() ) {
			clock_t end_clock = clock();
			elapsed_time += (float)(end_clock - begin_clock)/ CLOCKS_PER_SEC;
			double new_start_time = time_left - elapsed_time;
			int check_status = solve(new_start_time);
			if(check_status == -1) {
			    return -1;
			}
			
		}

		// If this assignment succeeded, return
		if ( hasSolution )
			return 0;

		// Otherwise backtrack
		trail->undo();
	}
	return 0;
}

bool BTSolver::checkConsistency ( void )
{
	if ( cChecks == "forwardChecking" )
		return forwardChecking().second;

	if ( cChecks == "norvigCheck" )
		return norvigCheck().second;

	if ( cChecks == "tournCC" )
		return getTournCC();

	return assignmentsCheck();
}

Variable* BTSolver::selectNextVariable ( void )
{
	if ( varHeuristics == "MinimumRemainingValue" )
		return getMRV();

	if ( varHeuristics == "MRVwithTieBreaker" )
		return MRVwithTieBreaker()[0];

	if ( varHeuristics == "tournVar" )
		return getTournVar();

	return getfirstUnassignedVariable();
}

vector<int> BTSolver::getNextValues ( Variable* v )
{
	if ( valHeuristics == "LeastConstrainingValue" )
		return getValuesLCVOrder( v );

	if ( valHeuristics == "tournVal" )
		return getTournVal( v );

	return getValuesInOrder( v );
}

bool BTSolver::haveSolution ( void )
{
	return hasSolution;
}

SudokuBoard BTSolver::getSolution ( void )
{
	return network.toSudokuBoard ( sudokuGrid.get_p(), sudokuGrid.get_q() );
}

ConstraintNetwork BTSolver::getNetwork ( void )
{
	return network;
}
