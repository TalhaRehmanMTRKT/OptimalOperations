#pragma once
#include <ilcplex/ilocplex.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


ILOSTLBEGIN

using namespace std;

typedef IloArray<IloNumVarArray> NumVar2D;


struct opt_res
{
	vector<double> p_short;
	vector<double> p_sur;
};




class CentralEMS
{
public:
	CentralEMS(int T, double* c_buy, double* c_sell, vector<opt_res> mgs_results);
	void run_optimization();
	double* c_buy = nullptr;
	double* c_sell = nullptr;
	vector<opt_res> mgs_results;
	int num_mgs = 0;
	int T = 0;
};



class Microgrid
{
public:
	Microgrid(int id, int T, double* c_buy, double* c_sell, string parameters_path, string rdg_path, string demand_path);
	// public variables of the class
	double* c_buy = nullptr;
	double* c_sell = nullptr;
	int T = 0;
	int id = NAN;
	opt_res get_results();

private:
	// private variables of the class
	vector<double> rdg;
	vector<double> demand;
	double battery_cap = 0;
	double battery_chg_max = 0;
	double battery_dch_max = 0;
	double battery_eff = 0;
	double battery_soc_min = 0;
	double battery_soc_max = 0;
	double dg_max = 0;
	double dg_min = 0;
	double dg_cost = 0;

	// private functions of the class
	void print_info();
	opt_res run_optimization();
};