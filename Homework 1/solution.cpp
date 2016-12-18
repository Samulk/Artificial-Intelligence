#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <queue>
#include <stack>

using namespace std;

struct Node {
	string location;
	Node* parent;
	int path_cost;
	int path_cost_ucs;
	//int heuristic_cost;
	int order_no;
	int depth;
	vector<Node*> children;
};

struct Compare
{
	 bool operator () ( const Node* a, const Node* b ) const 
    {
		if(a->path_cost < b->path_cost)
			return true;
		else if(a->path_cost > b->path_cost)
			return false;
		else if(a->path_cost == b->path_cost)
		{
			if(a->order_no < b->order_no)
			{
				return true;
			} else
				return false;
		}
		cout << "Code should never reach this statement!!!" << endl;
		cout << "Writing the line below it to get rid of the warning!!!" << endl;
		return false;
    }
};

struct find_in_set {
    find_in_set(const std::string & location) : location(location) {}
    bool operator()(const Node* const & n) {
		return n->location == location;
    }
private:
    std::string location;
};

struct Edge {
	int cost;
	string destination;
};

typedef map<string, vector<Edge> > Graph;

void buildGraph(ifstream& inFile, Graph& graph, map<string,int>& ordering, map<string,int>& sld);
void BFS(string start_state, string end_state, Graph graph, map<string,int> ordering);
void DFS(string start_state, string end_state, Graph graph, map<string,int> ordering);
void UCS(string start_state, string end_state, Graph graph, map<string,int> ordering);
void ASTAR(string start_state, string end_state, Graph graph, map<string,int> ordering, map<string,int> sld);

bool isMemberFrontier(vector<Node*> frontier, string destination);
int isMemberSetFrontier(set<Node* , Compare > frontier, string destination, int);
bool isMemberExplored(vector<Node*> explored, string destination);
int isMemberExploredAStar(vector<Node*> explored, string destination, int path_cost);

void printGraph(Graph g);
void printOrdering(map<string,int> ordering);
void printSLD(map<string,int> sld);
void printSolutionPath(Node* node);
void printSolutionPathAStar(Node* node);
void printFrontier(set<Node* , Compare > frontier);
void printExplored(vector<Node*> explored);
		
int main()
{
	ifstream inFile("input3.txt");
	string algo;
	if(inFile.is_open())
	{
		getline(inFile, algo);
		cout << "Algorithm: " << algo << endl;

		string start_state;     //starting point of our search
		getline(inFile, start_state);
		cout<<"Start State: " << start_state << endl;

		string end_state;        //destination where we want to reach
		getline(inFile, end_state);
		cout<<"End State: " << end_state << endl;

		map<string,int> ordering; //for mainting the order in which the cities are seen
		map<string, int> sld; //for storing the straight line distance for each city. To be used in A* search
		map<string,vector<Edge> > graph; //for maintaining all the info given in the graph
		buildGraph(inFile, graph, ordering, sld);
			
		if(algo == "BFS")
		{
			BFS(start_state, end_state, graph, ordering);
		} //end of BFS algo

		else if(algo == "DFS")
		{
			DFS(start_state, end_state, graph, ordering);
		} else if(algo == "UCS")
		{
			UCS(start_state, end_state, graph, ordering);
		} else if(algo == "A*")
		{
			ASTAR(start_state, end_state, graph, ordering, sld);
		}
		inFile.close();
	} else {
		cout << "File could not be opened!!!" << endl;
	}
	return 0;
} //end of main bracket
void printFrontier(set<Node* , Compare> frontier)
{
	cout <<"**********PRINTING FRONTIER***************"<<endl;
	set<Node* , Compare>::iterator itr = frontier.begin();
	for(; itr != frontier.end(); itr++)
	{
		Node* temp = (*itr);
		cout<< "City Name: " << temp->location << "\t\t" << "Total Path Cost: " << temp->path_cost << "\t\t" << "UCS Path: " << temp->path_cost_ucs << endl;
	}
}
void printExplored(vector<Node*> explored)
{
	cout <<"**********PRINTING EXPOLRED***************"<<endl;
	vector<Node*>::iterator itr = explored.begin();
	for(; itr != explored.end(); itr++)
	{
		Node* temp = (*itr);
		cout<< "City Name: " << temp->location << "\t\t" << "Total Path Cost: " << temp->path_cost << "\t\t" << "UCS Path: " << temp->path_cost_ucs << endl;
	}
}

void ASTAR(string start_state, string end_state, Graph graph, map<string,int> ordering, map<string,int> sld)
{
	set<Node* , Compare > frontier; //this will work as priority queue for this problem
									//have to make sure that priority is determined by pathcost + heuristic
									//however have to make sure that path cost in output is the same as UCS########################
	vector<Node*> explored;		//creating q for explored

	Node* n = new Node;			//creating the root node for the search tree
	n->location = start_state;
	n->depth = 0;
	n->parent = NULL;
	//n->heuristic_cost = sld[start_state];
	n->path_cost = 0 + sld[start_state];
	n->order_no = ordering[start_state];
	n->path_cost_ucs = 0;

	frontier.insert(n);		//inserting the first node in frontier

	while(1)
	{
		if(frontier.empty())
		{
			cout<< "Failure to find the destination....which is not possible!!!" << endl;
			return;
		}

		Node* temp = (*(frontier.begin()));		//taking the first node out of queue. Always remove the first node
		frontier.erase(frontier.begin());		//actually removing it from queue

		if(temp->location == end_state)			//if the popped node has the same state as our destination
		{
			cout<< "Found the destination!!!" << endl;	//we are done
			printSolutionPathAStar(temp);		//Modified this to print the cost for A*
			return;
		}

		explored.push_back(temp);				//since we have explored the node we put it in explored set

		vector<Edge> v = graph[temp->location];		//getting all the child of the current node

		for(int i = 0; i < (int)v.size(); i++)		//checking all the children nodes
		{
			int temp_path_cost = temp->path_cost_ucs + v[i].cost + sld[v[i].destination];
			//in the above line, the new cost = pathcost of ucs till here + cost of the edge + cost of the sld
			//have to make sure this is correct?????????????????????????????
			

			int flag1 = isMemberExploredAStar(explored, v[i].destination, temp_path_cost);
			//changing the method. We not only have to check for the presence of a node but also its value
			//bool flag1 = isMemberExplored(explored, v[i].destination); //What to do if it is in explored??????

			if(flag1 == 2)
				continue;
			else if(flag1 == 0)
			{
				//don't need to do anything, proceed normally
			} else if(flag1 == 1)
			{
				//found the node in the frontier but with a higher cost
				int difference = 0;
				int ucs_difference = 0;
				Node* t = NULL;
				//first have to update the parent and new cost for the node in the frontier
				vector<Node*>::iterator itr = explored.begin();
				for(; itr != explored.end(); itr++)
				{
					t = (*itr);
					if(t->location == v[i].destination)
					{
						t->depth = temp->depth + 1;
						t->parent = temp;
						ucs_difference = (t->path_cost_ucs - (temp->path_cost_ucs + v[i].cost)); 
						t->path_cost_ucs = temp->path_cost_ucs + v[i].cost;
						difference = t->path_cost - temp_path_cost;
						t->path_cost = temp_path_cost;
						break;
					}
				}

				//now check frontier, if there are any children of updated node, then 
				//update their path costs too
				//skipping that for now

				itr = explored.begin();
				for(; itr != explored.end(); itr++)
				{
					Node* texp = (*itr);
					if(texp->parent== t)
					{
						texp->path_cost -= difference;
						texp->path_cost_ucs -= ucs_difference;
					}
				}
				//now check the frontier, if there are any children of updated node, then
				//we will update their costs too.

				set<Node*, Compare>::iterator sitr = frontier.begin();

				for(; sitr != frontier.end(); sitr++)
				{
					Node* s = (*sitr);
					if(s->parent == t)
					{
						s->path_cost -= difference;
						s->path_cost_ucs -= ucs_difference;
					}
				}
				continue;
			}

			int result = isMemberSetFrontier(frontier, v[i].destination, temp_path_cost);

			if(result == 2)
				continue; //destination is in frontier and has higher cost, so don't need to do anything
			else if(result == 1) //destination is in frontier and has lower cost, so we need to update the path cost
			{
				std::set<Node* , Compare>::iterator result = std::find_if(frontier.begin(), frontier.end(), find_in_set(v[i].destination));

				if(result != frontier.end())
					frontier.erase(result);  //since the old node has greater path cost, we remove it
				else
					cout<<"NOT POSSIBLE TO HAPPEN!!!!" << endl;
			}
			//else if(result == 0) //not a member of frontier, so just add to it
			{
				//HAVE TO CHECK WHETHER THIS IS CORRECT OR NOT
				//============================================
				Node* child = new Node;
				child->parent = temp;
				child->depth = temp->depth + 1;
				child->location = v[i].destination;
				child->path_cost = temp_path_cost; //since this is BFS, all costs are uniform i.e. 1
				//child->heuristic_cost = 0;
				child->order_no = ordering[child->location];
				child->path_cost_ucs = temp->path_cost_ucs + v[i].cost;

				temp->children.push_back(child);
				
				frontier.insert(child);
			}
		}	//end of child node loop

		printExplored(explored);
		printFrontier(frontier);
		
	} //end of while 1 loop. we shall make sure that this loop terminates
}

int isMemberExploredAStar(vector<Node*> explored, string destination, int path_cost)
{
	vector<Node*>::iterator itr = explored.begin();
	for(; itr != explored.end(); itr++)
	{
		Node* temp = (*itr);
		if(temp->location == destination)
		{
			if(temp->path_cost >= path_cost)
			{
				return 1;//we need to replace this node
			} else {
				return 2; //don't need to do anything
			} 
		}
	}

	return 0; //not found in the explored set
}
void UCS(string start_state, string end_state, Graph graph, map<string,int> ordering)
{
	//priority_queue<Node*, std::vector<Node*>, Compare> frontier; //creating priority queue for frontier, algo is UCS
	set<Node* , Compare > frontier;	//this will work as priority queue for this problem.
									//here the priority is determined by path cost till now
	vector<Node*> explored;		//creating q for explored

	Node* n = new Node;			//creating the root node for the search tree
	n->location = start_state;
	n->depth = 0;
	n->parent = NULL;
	n->path_cost = 0;
	//n->heuristic_cost = 0;
	n->order_no = ordering[start_state];

	frontier.insert(n);		//inserting the first node in frontier

	while(1)
	{
		if(frontier.empty())
		{
			cout<< "Failure to find the destination....which is not possible!!!" << endl;
			return;
		}

		Node* temp = (*(frontier.begin()));
		frontier.erase(frontier.begin());

		if(temp->location == end_state)
		{
			cout<< "Found the destination!!!" << endl;
			//cout<< "Still have to find the path from start to finish!!!" << endl;
			printSolutionPath(temp);
			return;
		}

		explored.push_back(temp);

		vector<Edge> v = graph[temp->location];

		for(int i = 0; i < (int)v.size(); i++)
		{
			bool flag1 = isMemberExplored(explored, v[i].destination);

			if(flag1)
				continue;

			int temp_path_cost = temp->path_cost + v[i].cost;
			int result = isMemberSetFrontier(frontier, v[i].destination, temp_path_cost);

			if(result == 2)
				continue; //destination is in frontier and has higher cost, so don't need to do anything
			else if(result == 1) //destination is in frontier and has lower cost, so we need to update the path cost
			{
				std::set<Node* , Compare>::iterator result = std::find_if(frontier.begin(), frontier.end(), find_in_set(v[i].destination));

				if(result != frontier.end())
					frontier.erase(result);  //since the old node has greater path cost, we remove it
				else
					cout<<"NOT POSSIBLE TO HAPPEN!!!!" << endl;
			}
			//else if(result == 0) //not a member of frontier, so just add to it
			{
				//HAVE TO CHECK WHETHER THIS IS CORRECT OR NOT
				//============================================
				Node* child = new Node;
				child->parent = temp;
				child->depth = temp->depth + 1;
				child->location = v[i].destination;
				child->path_cost = temp->path_cost + v[i].cost; //since this is BFS, all costs are uniform i.e. 1
				//child->heuristic_cost = 0;
				child->order_no = ordering[child->location];
				temp->children.push_back(child);
				
				frontier.insert(child);
			}
		}
	}// end of while loop
}//end of UCS


int isMemberSetFrontier(set<Node* , Compare > frontier, string destination, int path_cost)
{
	set<Node* , Compare>::iterator result = std::find_if(frontier.begin(), frontier.end(), find_in_set(destination));

	if(result != frontier.end())
	{
		//found the destination in the frontier
		if((*result)->path_cost >= path_cost)
		{
			return 1;//we need to replace this node
		} else {
			return 2; //don't need to do anything
		}
	} else {
		//destination not found!!!
		return 0; //denotes not found
	}
}
void DFS(string start_state, string end_state, Graph graph, map<string,int> ordering)
{
	vector<Node*> frontier;		//creating q for frontier
	vector<Node*> explored;		//creating q for explored

	Node* n = new Node;			//creating the root node for the search tree
	n->location = start_state;
	n->depth = 0;
	n->parent = NULL;
	n->path_cost = 0;
	
	frontier.push_back(n);		//inserting the first node in frontier

	while(1)
	{
		if(frontier.empty())
		{
			cout<< "Failure to find the destination....which is not possible!!!" << endl;
			return;
		}

		Node* temp = frontier.front();
		frontier.erase(frontier.begin());

		if(temp->location == end_state)
		{
			cout<< "Found the destination!!!" << endl;
			//cout<< "Still have to find the path from start to finish!!!" << endl;
			printSolutionPath(temp);
			return;
		}

		explored.push_back(temp);

		vector<Edge> v = graph[temp->location];

		//for(int i = 0; i < (int)v.size(); i++)
		for(vector<Edge>::reverse_iterator rit = v.rbegin(); rit != v.rend(); ++rit)
		{
			bool flag1 = isMemberExplored(explored, (*rit).destination);
			bool flag2 = isMemberFrontier(frontier, (*rit).destination);

			if( !flag1 && !flag2) //not a member of explored or frontier
			{
				//HAVE TO CHECK WHETHER THIS IS CORRECT OR NOT
				//============================================
				Node* child = new Node;
				child->parent = temp;
				child->depth = temp->depth + 1;
				child->location = (*rit).destination;
				child->path_cost = temp->path_cost + 1; //since this is DFS, all costs are uniform i.e. 1
				temp->children.push_back(child);
				//frontier.push_back(child);
				frontier.insert(frontier.begin(),child); 
			}
		}

	}
} //end of DFS

void BFS(string start_state, string end_state, Graph graph, map<string,int> ordering)
{
	vector<Node*> frontier;		//creating q for frontier
	vector<Node*> explored;		//creating q for explored

	Node* n = new Node;			//creating the root node for the search tree
	n->location = start_state;
	n->depth = 0;
	n->parent = NULL;
	n->path_cost = 0;
	
	frontier.push_back(n);		//inserting the first node in frontier

	while(1)
	{
		if(frontier.empty())
		{
			cout<< "Failure to find the destination....which is not possible!!!" << endl;
			return;
		}

		Node* temp = frontier.front();
		frontier.erase(frontier.begin());

		if(temp->location == end_state)
		{
			cout<< "Found the destination!!!" << endl;
			//cout<< "Still have to find the path from start to finish!!!" << endl;
			printSolutionPath(temp);
			return;
		}

		explored.push_back(temp);

		vector<Edge> v = graph[temp->location];

		for(int i = 0; i < (int)v.size(); i++)
		{
			bool flag1 = isMemberExplored(explored, v[i].destination);
			bool flag2 = isMemberFrontier(frontier, v[i].destination);

			if( !flag1 && !flag2) //not a member of explored or frontier
			{
				//HAVE TO CHECK WHETHER THIS IS CORRECT OR NOT
				//============================================
				Node* child = new Node;
				child->parent = temp;
				child->depth = temp->depth + 1;
				child->location = v[i].destination;
				child->path_cost = temp->path_cost + 1; //since this is BFS, all costs are uniform i.e. 1
				temp->children.push_back(child);
				frontier.push_back(child);
			}
		}
	}
}

void printSolutionPathAStar(Node* node)
{
	stack<Node*> path;
	while(node != NULL)
	{
		path.push(node);
		node = node->parent;
	}

	ofstream outFile("output.txt");
	if(outFile.is_open())
	{
		while(!path.empty())
		{
			Node * top = path.top();
			outFile << top->location << " " << top->path_cost_ucs << endl;
			cout << top->location << " " << top->path_cost_ucs << endl;
			path.pop();
		}
	} else {
		cout<< "Output file could not be opened!!!" << endl;
	}
}
void printSolutionPath(Node* node)
{
	stack<Node*> path;
	while(node != NULL)
	{
		path.push(node);
		node = node->parent;
	}

	ofstream outFile("output.txt");
	if(outFile.is_open())
	{
		while(!path.empty())
		{
			Node * top = path.top();
			outFile << top->location << " " << top->path_cost << endl;
			cout << top->location << " " << top->path_cost << endl;
			path.pop();
		}
	} else {
		cout<< "Output file could not be opened!!!" << endl;
	}
}

bool isMemberExplored(vector<Node*> explored, string destination)
{
	bool flag = false;

	for(int i = 0; i < (int)explored.size(); i++)
	{
		if(explored[i]->location == destination)
		{
			return true;
		}
	}

	return false;
}

bool isMemberFrontier(vector<Node*> frontier, string destination)
{
	bool flag = false;

	for(int i = 0; i < (int)frontier.size(); i++)
	{
		if(frontier[i]->location == destination)
		{
			return true;
		}
	}

	return false;
}

void buildGraph(ifstream& inFile, Graph& graph, map<string,int>& ordering, map<string,int>& sld)
{
	
	int num_lines = 0;
	int index = 0;
	string line;
	getline(inFile,line);
	num_lines = std::stoi(line);

	cout << "Number of Live traffic lines: " << num_lines << endl;

	string source;		//this is the source for each edge in the graph
	string destination;	//this is the destination for each edge in the graph
	int cost;			//this is the cost for each edge in the graph
	for(int i = 0; i < num_lines; i++)
	{
		inFile>>source;
		inFile>>destination;
		inFile>>cost;
		//cout<<"From: " << source << " ----> To: " << destination<< "     Cost: " << cost << endl;

		if(ordering.find(source) == ordering.end())
		{
			ordering[source] = index;
			index++;
		}

		if(ordering.find(destination) == ordering.end())
		{
			ordering[destination] = index;
			index++;
		}
	
		Edge e;
		e.destination = destination;
		e.cost = cost;

		graph[source].push_back(e);
	}

	getline(inFile,line);
	getline(inFile,line);
	num_lines = std::stoi(line);

	cout << "Number of Sunday traffic lines: " << num_lines << endl;
	for(int i = 0; i < num_lines; i++)
	{
		inFile>>source; //name of the city
		inFile>>cost;   //cost of straight line distance to the destination
		
		if(sld.find(source) == sld.end())
		{
			sld[source] = cost;
		} else {
			cout << "This output would imply that there are 2 cities with the same name in Sunday traffic!!!" << endl;
			cout << "This is not supposed to happen!!!" << endl;
		}

	}
	//printGraph(graph);
	//printOrdering(ordering);
	//printSLD(sld);
}

void printGraph(Graph g)
{
	map<string,vector<Edge> >::iterator itr;
	for(itr = g.begin(); itr != g.end(); itr++)
	{
		cout <<itr->first << " ----> ";
		vector<Edge>::iterator vitr = itr->second.begin();
		for(;vitr != itr->second.end(); vitr++)
		{
			cout << (*vitr).destination << ", ";
		}
		cout << endl;
	}
}

void printOrdering(map<string,int> ordering)
{
	map<string,int>::iterator itr;
	for(itr = ordering.begin(); itr != ordering.end(); itr++)
	{
		cout<< itr ->first << " " << itr->second << endl;
	}
}

void printSLD(map<string,int> sld)
{
	map<string,int>::iterator itr;
	for(itr = sld.begin(); itr != sld.end(); itr++)
	{
		cout<< "Name of City: " << itr ->first << "       SLD Cost: " << itr->second << endl;
	}
}
