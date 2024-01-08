#include <ilcplex/ilocplex.h>
#include<chrono>
#include <iostream>
#include <fstream>
ILOSTLBEGIN


int
main(int, char**)
{

    auto start = chrono::high_resolution_clock::now();
    IloEnv env;
    IloModel model(env);

#pragma region Microgrid Input Data
    int T = 24; //One day

    int* Pload = new int[T] {169, 175, 179, 171, 181, 172, 270, 264, 273, 281, 193, 158, 161, 162, 250, 260, 267, 271, 284, 167, 128, 134, 144, 150};  //Electicity demand w.r.t tim
    int* CGbuy = new int[T] { 90, 90, 90, 90, 90, 90, 110, 110, 110, 110, 110, 125, 125, 125, 125, 125, 125, 125, 110, 110, 110, 110, 110, 110 };  //buying price from grid w.r.t time

    int Pbmax = 200; //battery maximum capacity
    int chgmax = 100; //battery maximum charging rate
    int dischgmax = 100; //battery maximum discharging rate
    
    float chgeffin = 0.95; //battery effciency
    float dischgeffin = 0.95; //battery effciency
#pragma endregion


#pragma region Decision Variables
    IloNumVarArray PGbuy(env, T, 0, IloInfinity);//Grid power bought
    IloNumVarArray PGsell(env, T, 0, IloInfinity);//Grid power sold
    IloNumVarArray statoc(env, T, 0, 1); //battery storage capacity
    IloNumVarArray Bchg(env, T, 0, IloInfinity); //battery charging
    IloNumVarArray Bdischg(env, T, 0, IloInfinity); //battery discharging
#pragma endregion


#pragma region Objective Function
    IloExpr objective(env); //Defining the expression for the objective function
    for (int t = 0; t < T; t++)
    {
        // sum of cost incurred by buying power from grid at each time step
        objective += CGbuy[t] * PGbuy[t];
    }
    // Objective: minimize cost over 24 hours
    model.add(IloMinimize(env, objective));
#pragma endregion


#pragma region Constraints
    // Constraint: meet demand
    for (int t = 0; t < T; t++)
    {
        // Adding the Battery Constraints constraints
        if (t == 0)
        {
            model.add(statoc[t] == statoc[T-1] + ((chgeffin * Bchg[t] - (Bdischg[t] / dischgeffin)) / Pbmax));
            model.add(0 <= Bchg[t]);
            model.add(0 <= Bdischg[t]);
            model.add(Bchg[t] <= (Pbmax * (1 - statoc[T - 1]) / chgeffin));
            model.add(Bdischg[t] <= (Pbmax * statoc[T - 1] * dischgeffin));
        }
        else
        {
            model.add(statoc[t] == statoc[t - 1] + ((chgeffin * Bchg[t] - (Bdischg[t] / dischgeffin)) / Pbmax));
            model.add(0 <= Bchg[t]);
            model.add(0 <= Bdischg[t]);
            model.add(Bchg[t] <= (Pbmax * (1 - statoc[t - 1])) / chgeffin);
            model.add(Bdischg[t] <= Pbmax * statoc[t - 1] * dischgeffin);
        }

        model.add(Bchg[t] <= chgmax);
        model.add(Bdischg[t] <= dischgmax);
        model.add(0 <= statoc[t]);
        model.add(statoc[t] <= 1);

        // Adding the power balance constraint
        model.add(Bdischg[t] + PGbuy[t] == Pload[t] + Bchg[t]);
    }
#pragma endregion


#pragma region Results
    IloCplex cplex(env);
    cplex.extract(model);
    cplex.setOut(env.getNullStream());
    if (!cplex.solve()) {
        env.error() << "Failed" << endl;
        throw(-1);
    }
    double obj = cplex.getObjValue();
    auto end = chrono::high_resolution_clock::now();
    auto Elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "\n\n\t The elapsed time is: \t" << Elapsed.count() << endl;
    cout << "Solution status: " << cplex.getStatus() << endl;
    cout << "Minimized Objective Funtion : " << obj << endl;
    IloNum eps = cplex.getParam(
        IloCplex::Param::MIP::Tolerances::Integrality);
#pragma endregion


#pragma region StoringOutput
    // Create and open the CSV file for writing
    std::ofstream outputFile("output.csv");

    if (outputFile.is_open()) {
        // Write the header row
        outputFile << "Time,Pload,CGbuy,PGbuy,statoc,Bchg,Bdischg" << std::endl;

        // Write data to a CSV row
        for (int i = 0; i < T; i++)
        {
            outputFile << i + 1 << "," << Pload[i] << "," << CGbuy[i] << "," << cplex.getValue(PGbuy[i]) << "," << cplex.getValue(statoc[i]) << "," << cplex.getValue(Bchg[i]) << "," << cplex.getValue(Bdischg[i]) << std::endl;
        }

        // Close the CSV file
        outputFile.close();
        std::cout << "Data saved to output.csv" << std::endl;
    }
    else {
        std::cerr << "Failed to open the output.csv file for writing." << std::endl;
    }
#pragma endregion

    env.end();
    system("pause");
    return 0;
}