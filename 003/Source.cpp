#include <ilcplex/ilocplex.h>
#include <iostream>
#include <fstream>
ILOSTLBEGIN

using namespace std;

int main() {


#pragma region constants
    const int M = 10000; // Big M
    const int T = 24; // Optimization horizon (hours)
    const int battery_cap = 500; // Capacity of battery (kWh)
    const int battery_chg_max = 250; // Maximum charging rate of battery (kWh)
    const int battery_dch_max = 250; // Maximum discharging rate of battery (kWh)
    const int water_tank_cap = 15; // Capacity/Height of water tank (m)
    const int water_tank_chg_max = 75; // Maximum charging rate of water tank (m^3)
    const int water_tank_dch_max = 75; // Maximum discharging rate of water tank (m^3)
    const int heigt_pipe = 3; // Height of pipe (m)
    const int water_tank_area = 10; // Area of water tank (m^2)
    const int desalination_cap = 300; // Maximum power of water pump (m^3)
    const float battery_charge_eff = 0.95; // Charging efficiency 
    const float battery_discharge_eff = 0.95; // Discharging efficiency
    const float g = 9.8; // acceleration due to gravity (m/s^2)
    const float rho = 997; // density of water (kg/m^3)
    const float water_tank_charge_eff = 0.99; // Charging efficiency 
    const float water_tank_discharge_eff = 0.99; // Discharging efficiency
    const float desalination_eff = 0.3295; // Desalination efficiency
    const float water_pump_eff = 0.85; // Water pump efficiency 
    const float* c_buy = new float[T] { 90, 90, 90, 90, 90, 90, 110, 110, 110, 110, 110, 125, 125, 125, 125, 125, 125, 125, 110, 110, 110, 110, 110, 110 }; // Cost of buying electricity (cents/kWh) w.r.t time
    const float* c_sell = new float[T] { 70, 70, 70, 70, 70, 70, 90, 90, 90, 90, 90, 105, 105, 105, 105, 105, 105, 105, 90, 90, 90, 90, 90, 90}; //selling price to grid (cents/kWh) w.r.t time
    const float* electricity_demand = new float[T] { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 150, 150, 150, 150, 150, 150, 150, 150, 100, 100, 100, 100, 100, 100 }; // Electricity demand (kWh)
    const float* water_demand = new float[T] { 50, 50, 80, 80, 90, 90, 90, 100, 100, 100, 120, 120, 120, 120, 120, 120, 120, 50, 50, 80, 80, 90, 90, 90}; // Water demand (m^3)
#pragma endregion



    IloEnv env;
    IloModel model(env);


#pragma region Decision variables
    // Electricity Network
    IloNumVarArray p_buy(env, T, 0, IloInfinity, ILOFLOAT); // Power bought from grid
    IloNumVarArray p_sell(env, T, 0, IloInfinity, ILOFLOAT); // Power sold to grid
    IloNumVarArray p_battery_chg(env, T, 0, IloInfinity, ILOFLOAT); // Power charging battery
    IloNumVarArray p_battery_dch(env, T, 0, IloInfinity, ILOFLOAT); // Power discharging battery
    IloNumVarArray soc_battery(env, T, 0, IloInfinity, ILOFLOAT); // State of charge of battery
    IloNumVarArray p_desalination(env, T, 0, IloInfinity, ILOFLOAT); // Power used for desalination
    IloNumVarArray p_water_pump(env, T, 0, IloInfinity, ILOFLOAT); // Power used for water pump
    
    // Water Network
    IloNumVarArray water_tank_chg(env, T, 0, IloInfinity, ILOFLOAT); // water tank charging
    IloNumVarArray water_tank_dch(env, T, 0, IloInfinity, ILOFLOAT); // water tank discharging
    IloNumVarArray water_tank_level(env, T, 0, IloInfinity, ILOFLOAT); // water tank level
    IloNumVarArray water_desalination(env, T, 0, IloInfinity, ILOFLOAT); // water desalination

    // Binary variable
    IloNumVarArray omega(env, T, 0, IloInfinity, ILOINT);
    IloNumVarArray nu(env, T, 0, IloInfinity, ILOINT);

    //IloNumVarArray omega(env, T, 0, 1, ILOINT);
    //IloNumVarArray nu(env, T, 0, 1, ILOINT);


    //Mccormick variable
    IloNumVarArray z(env, T, 0, IloInfinity, ILOFLOAT); // Mccormick variable

#pragma endregion


#pragma region Objective function
    // Cost minimization
    IloExpr objective(env);
    for (int t = 0; t < T; t++) {
		objective += c_buy[t] * p_buy[t] - c_sell[t] * p_sell[t];
	}
	model.add(IloMinimize(env, objective));
#pragma endregion


#pragma region Constraints


    for (int t = 0; t < T; t++)
    {
        // Limiting constraints
        model.add(p_battery_chg[t] <= battery_chg_max);
        model.add(p_battery_dch[t] <= battery_dch_max);
        model.add(water_tank_chg[t] <= water_tank_chg_max);
        model.add(water_tank_dch[t] <= water_tank_dch_max);
        model.add(water_desalination[t] <= desalination_cap);
        model.add(soc_battery[t] <= battery_cap);
        model.add(water_tank_level[t] <= water_tank_cap);
        model.add(omega[t] <= 1);
        model.add(nu[t] <= 1);

        // Non-negativity
        model.add(p_buy[t] >= 0);
        model.add(p_sell[t] >= 0);
        model.add(p_battery_chg[t] >= 0);
        model.add(p_battery_dch[t] >= 0);
        model.add(soc_battery[t] >= 0);
        model.add(p_desalination[t] >= 0);
        model.add(p_water_pump[t] >= 0);
        model.add(water_tank_chg[t] >= 0);
        model.add(water_tank_dch[t] >= 0);
        model.add(water_tank_level[t] >= 0);
        model.add(water_desalination[t] >= 0);
        model.add(z[t] >= 0);
        model.add(omega[t] >= 0);
        model.add(nu[t] >= 0);





        // power balance for electricity
        model.add(p_buy[t] + p_battery_dch[t] - p_desalination[t] - p_water_pump[t]  - p_sell[t] - p_battery_chg[t] == electricity_demand[t]);
        // power balance for water
        model.add(water_desalination[t] + water_tank_dch[t] - water_tank_chg[t] == water_demand[t]);

        // Big M constraints to ensure that the charging and discharging not happen at the same time
        model.add(p_battery_chg[t] <= M * omega[t]);
        model.add(p_battery_dch[t] <= M * (1-omega[t]));
        model.add(water_tank_chg[t] <= M * nu[t]);
        model.add(water_tank_dch[t] <= M * (1-nu[t]));


        
        // Operational constraints
        model.add(water_desalination[t] == desalination_eff * p_desalination[t]);
        model.add(p_water_pump[t] ==  ((water_tank_dch[t]*water_tank_cap - z[t] + water_tank_dch[t]*heigt_pipe)*g*rho) / (water_pump_eff*3.6*1e6));


        // McCormick constraints
        model.add(z[t] >=  0*water_tank_level[t] + water_tank_dch[t]*0 - 0*0);
        model.add(z[t] >=  water_tank_dch_max * water_tank_level[t] + water_tank_dch[t] * water_tank_cap - water_tank_dch_max* water_tank_cap);

        model.add(z[t] <= water_tank_dch_max * water_tank_level[t] + water_tank_dch[t] * 0 - water_tank_dch_max* 0);
        model.add(z[t] <= water_tank_dch[t] * water_tank_cap + water_tank_level[t] * 0 - 0 * water_tank_cap);

        // soc constraints
        if (t == 0) {
			model.add(soc_battery[t] == soc_battery[T - 1] + battery_charge_eff * p_battery_chg[t] - p_battery_dch[t] / battery_discharge_eff);
            model.add(water_tank_level[t] == water_tank_level[T - 1] + water_tank_charge_eff * water_tank_chg[t]/water_tank_area - water_tank_dch[t] / (water_tank_discharge_eff*water_tank_area));
		}
        else {
			model.add(soc_battery[t] == soc_battery[t - 1] + battery_charge_eff * p_battery_chg[t] - p_battery_dch[t] / battery_discharge_eff);
            model.add(water_tank_level[t] == water_tank_level[t - 1] + water_tank_charge_eff * water_tank_chg[t]/water_tank_area - water_tank_dch[t] / (water_tank_discharge_eff*water_tank_area));
		}

    }
#pragma endregion

    // Solving the model
    IloCplex cplex(model);
    cplex.exportModel("model.lp");
    cplex.solve();
    cout << "Objective function value: " << cplex.getObjValue() << endl;


    // Saving the results to a csv file
    ofstream file("results.csv");
    file << "p_buy,p_sell,p_battery_chg,p_battery_dch,soc_battery,p_desalination,p_water_pump,water_tank_chg,water_tank_dch,water_tank_level,water_desalination,z,water_demand,electricity_demand,c_buy,c_sell,water_tank_dch*water_tank_level" << endl;
    for (int t = 0; t < T; t++) {
		file << cplex.getValue(p_buy[t]) << "," << - cplex.getValue(p_sell[t]) << "," << - cplex.getValue(p_battery_chg[t]) 
            << "," << cplex.getValue(p_battery_dch[t]) << "," << cplex.getValue(soc_battery[t]) << "," 
            << - cplex.getValue(p_desalination[t]) << "," << - cplex.getValue(p_water_pump[t]) << "," 
            <<- cplex.getValue(water_tank_chg[t]) << "," << cplex.getValue(water_tank_dch[t]) << ","
            << cplex.getValue(water_tank_level[t]) << "," << cplex.getValue(water_desalination[t]) 
            << "," << cplex.getValue(z[t]) << "," << water_demand[t] << "," << electricity_demand[t]
            << "," << c_buy[t] << "," << c_sell[t]
            << "," << cplex.getValue(water_tank_dch[t]) * cplex.getValue(water_tank_level[t]) << endl;
	}


    return 0;
}
