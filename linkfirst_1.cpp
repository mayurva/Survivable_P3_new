#include <fstream>
#include <sstream>
#include <string>
#include <time.h>
#include <ilcplex/ilocplex.h>
  ILOSTLBEGIN                                            //a macro that is needed for portability (necessary) 

typedef IloArray<IloNumArray> Xjk;
typedef IloArray<Xjk> Xijk;
typedef IloArray<Xijk> Xijkl; //xijkl[l][i][j][k]
FILE *file;

void input_Xijkl(Xijkl xijkl_m){
	file = fopen("Proj3_processed.txt","r");
        if(!file){
                perror("Cannot Open File\n");
                exit(-1);
        }
        fseek(file,0,SEEK_SET);


    int i,j,k,l;	
    while(fscanf(file,"%d %d %d %d\n",&l,&i,&j,&k)!= EOF){
	xijkl_m[l][i][j][k] = 1;
    }
    fclose(file);
}


int  main (int argc, char *argv[])
{ 
     ifstream infile;
     clock_t start_time, end_time;
     infile.open("Proj3_op.txt");
     if(!infile){
	cerr << "Unable to open the file\n";
	exit(1);
     }
     
     cout << "Before Everything!!!" << "\n";
     IloEnv env;
     IloInt   i,j,varCount1,varCount2,varCount3,conCount;                                                    //same as �int i;�
     IloInt k,w,K,W,E,l,P,N,L;
     IloInt tab, newline, val; //from file
     char line[2048];
     try {
	N = 9;
	K = 2;
	L = 36;
	W = (IloInt)atoi(argv[1]);
        IloModel model(env);		//set up a model object

	IloNumVarArray var1(env);// = IloNumVarArray(env,K*W*N*N);
//	IloNumVarArray var2(env);
	IloNumVarArray var3(env);// = IloNumVarArray(env,W);		//declare an array of variable objects, for unknowns 
	IloNumVar W_max(env, 0, W, ILOINT);
	//var1: c_ijk_w
	//var2: X_ijk_l
	IloRangeArray con(env);// = IloRangeArray(env,N*N + 3*w);		//declare an array of constraint objects
        IloNumArray2 t = IloNumArray2(env,N); //Traffic Demand
        IloNumArray2 e = IloNumArray2(env,N); //edge matrix
        //IloObjective obj;

	//Define the Xijk matrix
     	Xijkl xijkl_m(env, L);
        for(l=0;l<L;l++){
                xijkl_m[l] = Xijk(env, N);
                for(i=0;i<N;i++){
                        xijkl_m[l][i] = Xjk(env, N);
                        for(j=0;j<N;j++){
                                xijkl_m[l][i][j] = IloNumArray(env, K);
                        }
                }
        }


	
	//reset everything to zero here
	for(l=0;l<L;l++)
                for(i=0;i<N;i++)
                        for(j=0;j<N;j++)
                                for(k=0;k<K;k++)
                                        xijkl_m[l][i][j][k] = 0;

	input_Xijkl(xijkl_m);


	
	cout<<"bahre\n";
	
	for(i=0;i<N;i++){
		t[i] = IloNumArray(env,N);
		for(j=0;j<N;j++){
			if(i == j)
				t[i][j] = IloNum(0);
			else if(i != j)
				t[i][j] = IloNum(3);
		}
	}
	
	printf("ikde\n");
	//Minimize W_max
        IloObjective obj=IloMinimize(env);
	obj.setLinearCoef(W_max, 1.0);

	cout << "here khali\n"; 
	//Setting var1[] for Demands Constraints
	for(i=0;i<N;i++)
		for(j=0;j<N;j++)
			for(k=0;k<K;k++)
				for(w=0;w<W;w++)
					var1.add(IloNumVar(env, 0, 1, ILOINT));


	for(w = 0;w < W;w++)
		var3.add(IloNumVar(env, 0, 1, ILOINT)); //Variables for u_w
	cout<<"variables ready\n";
	conCount = 0;
	for(i=0;i<N;i++)
		for(j=0;j<N;j++){
			con.add(IloRange(env, t[i][j], t[i][j]));
			//varCount1 = 0;
			for(k=0;k<K;k++)
				for(w=0;w<W;w++){
					con[conCount].setLinearCoef(var1[i*N*W*K+j*W*K+k*W+w],1.0);
					//cout << "Before Adding Constraint\n";
					//con[1].setLinearCoef(IloNumVar(env, 0, 1, ILOINT), 1.0);
					//cout<<"coef set "<<varCount1;
				}
			conCount++;
		}//Adding Demands Constraints to con
	cout<<"1st\n";

	IloInt z= 0;
        for(w=0;w<W;w++){
                for(l=0;l<L;l++){
                        con.add(IloRange(env, -IloInfinity, 1));
                        for(i=0;i<N;i++){
                                for(j=0;j<N;j++){
                                        for(k=0;k<K;k++){
                                                con[conCount].setLinearCoef(var1[i*N*W*K+j*W*K+k*W+w],xijkl_m[l][i][j][k]);
                                        }
                                }
                        }
                        conCount++;
                }
        }


	cout<<"2nd\n";

	//Adding Wavelength Constraints_1 to con
	P = N * (N-1) * K;	
	for(w=0;w<W;w++){
		con.add(IloRange(env, -IloInfinity, 0));
		varCount1 = 0;
                for(i=0;i<9;i++)
                       for(j=0;j<9;j++)
                               for(k=0;k<K;k++){
					con[conCount].setLinearCoef(var1[i*N*W*K+j*W*K+k*W+w],1.0);
                               }
		con[conCount].setLinearCoef(var3[w],-P);
                conCount++;

	}
	cout<<"3rd\n";
	
	varCount3 = 0;
	for(w=0;w<W;w++){
		con.add(IloRange(env, 0, IloInfinity));
		con[conCount].setLinearCoef(W_max, 1.0);
 		con[conCount++].setLinearCoef(var3[w], -1.0 * (w+1));
	}
	cout<<"after constraints\n";

	
	//model.add(obj);			//add objective function into model
        model.add(IloMinimize(env,obj));
	model.add(con);			//add constraints into model
        IloCplex cplex(model);			//create a cplex object and extract the 					//model to this cplex object
        // Optimize the problem and obtain solution.
	start_time = clock();
        if ( !cplex.solve() ) {
           env.error() << "Failed to optimize LP" << endl;
           throw(-1);
        }
	end_time = clock();
        IloNumArray vals(env);		//declare an array to store the outputs
	IloNumVarArray opvars(env);			 //if 2 dimensional: IloNumArray2 vals(env);
        //env.out() << "Solution status = " << cplex.getStatus() << endl;
		//return the status: Feasible/Optimal/Infeasible/Unbounded/Error/�
        env.out() << "W_max value  = " << cplex.getObjValue() << endl; 
		//return the optimal value for objective function
        cplex.getValues(vals, var1);			//get the variable outputs
        //env.out() << "Values Var1        = " <<  vals << endl;	//env.out() : output stream
	cplex.getValues(vals, var3);
	//env.out() << "Values Val3        = " <<  vals << endl; 

     }
     catch (IloException& e) {
        cerr << "Concert exception caught: " << e << endl;
     }
     catch (...) {
        cerr << "Unknown exception caught" << endl;
     }
  
     env.end();				//close the CPLEX environment
     float running_time (((float)end_time - (float)start_time)/CLOCKS_PER_SEC);
     cout << "*******RUNNING TIME: " << running_time << endl;
     return 0;
  }  // END main
