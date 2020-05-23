#include "Circuit.hpp"
#include "Component.hpp"
#include "Parser.hpp"
#include<Eigen/Dense>


//helper functions for Matrix_solver
void remove_Row(Eigen::MatrixXd& matrix, unsigned int rowToRemove)
{
    unsigned int numRows = matrix.rows()-1;
    unsigned int numCols = matrix.cols();

    if( rowToRemove < numRows )
        matrix.block(rowToRemove,0,numRows-rowToRemove,numCols) = matrix.block(rowToRemove+1,0,numRows-rowToRemove,numCols);

    matrix.conservativeResize(numRows,numCols);
}

void remove_Column(Eigen::MatrixXd& matrix, unsigned int colToRemove)
{
    unsigned int numRows = matrix.rows();
    unsigned int numCols = matrix.cols()-1;

    if( colToRemove < numCols )
        matrix.block(0,colToRemove,numRows,numCols-colToRemove) = matrix.block(0,colToRemove+1,numRows,numCols-colToRemove);

    matrix.conservativeResize(numRows,numCols);
}


Eigen::VectorXd Matrix_solver(const Circuit& input_circuit)
{ 
    //initializing a matrix of the right size 
    int Mat_size = input_circuit.get_number_of_nodes();     
    Eigen::MatrixXd Mat(Mat_size,Mat_size);

    //initializing a vector of the right size
    Eigen::VectorXd Vec(Mat_size);

    //Fills up the matrix with 0s
    Mat.Zero(Mat_size,Mat_size);

    //Fills up the vector with 0s
    Vec.Zero();


    //nodes needed to set up the KCL equations
    std::vector<Node> nodes = input_circuit.get_nodes();            //possible optimization here

    //needed to store voltage components
    std::vector<Voltage_Source*> voltage_sources;

    //initializing each element of the conductance matrix
    //sum of currents out of a node = 0
    for (int row = 0; row < Mat_size; row++)
    {
        int node_index = row;

        //iterating through each componenet of the node considered
        for(Component* component_attached: nodes[node_index].components_attached)
        {
            //checking type of each component and act accordingly
            if(dynamic_cast<Resistor*>(component_attached))
            {
                int anode = component_attached->get_anode();
                int cathode = component_attached->get_cathode();
                Resistor* Rptr = dynamic_cast<Resistor*>(component_attached);

                if(anode == node_index)
                {
                    Mat(node_index,anode)+= Rptr->get_conductance();
                    Mat(node_index,cathode)-= Rptr->get_conductance();
                }
                else if (cathode == node_index)
                {
                    Mat(node_index,cathode)+= Rptr->get_conductance();
                    Mat(node_index,anode)-= Rptr->get_conductance();
                }
            }
            if(dynamic_cast<Current_source*>(component_attached))
            {
                int anode = component_attached->get_anode();
                int cathode = component_attached->get_cathode();
                Current_source* Currentptr = dynamic_cast<Current_source*>(component_attached);

                if(anode == node_index)
                {
                    Vec(node_index) += Currentptr->get_current();
                }
                else if (cathode == node_index)
                {
                    Vec(node_index) -= Currentptr->get_current();
                }
            }
            if(dynamic_cast<Voltage_Source*>(component_attached))
            {
                int anode = component_attached->get_anode();
                int cathode = component_attached->get_cathode();
                Voltage_Source* Vptr = dynamic_cast<Voltage_Source*>(component_attached);

                //needs to be processed at the end
                voltage_sources.push_back(Vptr);
            }
        }
    }
        //processing voltage sources
    for(Voltage_Source* voltage_source : voltage_sources)
    {
        int anode = voltage_source->get_anode();
        int cathode = voltage_source->get_cathode();
        double voltage = voltage_source->get_voltage();

        if(cathode == 0)
        {   
            Mat.row(anode).setZero();
            Mat(anode,anode) = 1;                   
            Vec(anode) = voltage;
        }
        else if (anode == 0)
        {
            Mat.row(cathode).setZero();
            Mat(cathode,cathode) = -1;
            Vec(cathode) = voltage;
        }
        else
        {   
            //combines KCL equations
            Mat.row(cathode) += Mat.row(anode);
            Vec(cathode) += Vec(anode);
            //sets anode node to V_anode - V_cathode = deltaV
            Mat.row(anode).setZero();
            Mat(anode,anode) = 1;
            Mat(anode,cathode) = -1;
            Vec(anode) = voltage;
        }
    }
    //setting V0 to GND and removing corresponding row and column
    /* todo
    remove_Row(Mat,0);
    remove_Column(Mat,0);
    remove_Row(Vec,0);
    */
    //finding the inverse matrix
    Eigen::VectorXd solution(Mat_size -1);
    solution = Mat.colPivHouseholderQr().solve(Vec);
    return solution;

}



 

