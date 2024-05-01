#include <ilcplex/ilocplex.h>

ILOSTLBEGIN

int main() {
    IloEnv env;
    try {
        // Create an instance of the CPLEX environment
        IloModel model(env);

        // Create decision variables
        IloNumVar x1(env, 0, IloInfinity, ILOFLOAT);
        IloNumVar x2(env, 0, IloInfinity, ILOFLOAT);

        //objective function

        model.add(IloMaximize(env, 5 * x1 + 4 * x2));

        // Add constraints
        model.add(6 * x1 + 4 * x2 <= 24);
        model.add(x1 + 2 * x2 <= 6);
        model.add(-x1 + x2 <= 1);
        model.add(x2 <= 2);
        model.add(x2 >= 0);
        model.add(x1 >= 0);

        // Create a CPLEX solver instance
        IloCplex cplex(model);

        // Solve the model
        cplex.solve();

        // Print the solution
        cout << "Objective Value: " << cplex.getObjValue() << endl;
        cout << "Solution:" << endl;
        cout << "x1 = " << cplex.getValue(x1) << endl;
        cout << "x2 = " << cplex.getValue(x2) << endl;

    }
    catch (IloException& e) {
        cerr << "Error: " << e << endl;
    }
    env.end();

    return 0;
}