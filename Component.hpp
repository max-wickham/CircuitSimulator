#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include<iostream>
#include<string>
#include<cmath>

class Component
{
protected:
    std::string name;
    char type = 'A';
    int anode;
    int cathode;
    double value;
    

public:
    Component(){}                            //to implement
    virtual ~Component(){}

    /*
    reurns the type of the component
    */
    virtual char get_type() const {return type;}

    /*
    reurns the anode of the component
    */     
    int get_anode() const {return anode;}

    /*
    reurns the cathode of the component
    */  
    int get_cathode() const {return cathode;}
    double get_value() const {return value;}
    std::string get_name() const {return name;}
};



class Current_Component:
    public Component
{

public:
    /*
    given a vector<double> nodevoltages where 
    -each index of nodevoltages corresponds to a node
    -each double of nodevoltages corresponds to a voltage at the given node
    returns the current through the Component going from anode to cathode
    */ 
    virtual double get_current(const std::vector<double> &nodevoltages) const = 0;

    /*
    given a vector<double> nodevoltages where 
    -each index of nodevoltages corresponds to a node
    -each double of nodevoltages corresponds to a voltage at the given node
    returns a vector<double> current_derivative where 
    -each index of current_derivative corresponds to a node
    -each double of current_derivative corresponds to the partial derivative of the current through the component (from characteristic equation) with respect to the corresponding voltage provided by nodevoltages
    */ 
    virtual std::vector<double> get_current_derivative(const std::vector<double> &nodevoltages) const = 0;
};






class Resistor :
    public Current_Component
{
public:
    Resistor(int _anode, int _cathode, std:: string _name, double _value){
        anode = _anode;
        cathode = _cathode;
        name = _name;
        value = _value;
    }
    ~Resistor(){}

    char get_type(){
        return 'R';
    }
    double get_current(const std::vector<double> &nodevoltages) const override
    {
        double current = (nodevoltages[anode]-nodevoltages[cathode])/value;
        //std::cout << "Resistor Current" << current << std::endl;
        return current;
    }

    std::vector<double> get_current_derivative(const std::vector<double> &nodevoltages) const override
    {
        std::vector<double> current_derivative(nodevoltages.size(),0);
        current_derivative[anode]= 1/value;
        current_derivative[cathode]= -1/value;
        return current_derivative;
    }


};






class Inductor :
    public Current_Component
{
private:
    /*
    integral of voltage wrt time. Voltage is defined as V_anode - V_cathode
    */
    double integral ;
public:
    Inductor(){}
    ~Inductor(){}
    /*
    given an increment computed by "controller" due to a timestep
    updates the value of Inductor::integral
    */
    void update_integral(double increment){
        integral += increment;
    }
    
    double get_current(const std::vector<double> &nodevoltages) const override        //does not need the parameter. modify later
    {
        return integral/value;
    }

    std::vector<double> get_current_derivative(const std::vector<double> &nodevoltages)         //to implement. needs controller to know the previous value of current
    {
        //std::cerr << "not implemented yet" << std::endl;
        std::vector<double> current_derivative(nodevoltages.size(),0);
        return current_derivative;
    }

};



class Current_source :
    public Current_Component
{
private:
    /*
    fixed current going from anode to cathode
    */
    double current;
public:
    Current_source(int _anode, int _cathode, std:: string _name, double _value){
        anode = _anode;
        cathode = _cathode;
        name = _name;
        value = _value;
    }
    ~Current_source(){}

    double get_current(const std::vector<double> &nodevoltages) const override
    {
        return current;
    }

    std::vector<double> get_current_derivative(const std::vector<double> &nodevoltages) const override
    {
        std::vector<double> current_derivative(nodevoltages.size(),0);
        return current_derivative;
    }
};    



class Diode:
    public Current_Component
{
private:
    double Vt = 0.025;
    double I_s = 0.0000000000001;
    double vd = 0;
    double id0 = 0;
public:
    Diode(int _anode, int _cathode, std:: string _name, double _value){
        anode = _anode;
        cathode = _cathode;
        name = _name;
        value = _value;
    }
    ~Diode(){}
    void set_vd(double _vd){
        vd = _vd;
    }
    void set_id0(double _id0){
        id0 = _id0;
    }
    double get_conductance(){
        double conductance = ((I_s/Vt)*exp(vd/Vt));
        conductance = conductance<0.01?0.01:conductance;
        conductance = conductance>10?10:conductance;
        std::cout << "conductance " << conductance << std::endl;
        std::cout << "Vd " << vd << std::endl;
        return conductance;
    }
    double get_linear_current(){
        std::cout << "current " << (id0 - ((I_s/Vt)*exp(vd/Vt))*vd) << std::endl;
        double current = (id0 - this->get_conductance()*vd);
        current = current>10?10:current;
        return current;
    }
    double get_current(const std::vector<double> &nodevoltages) const override
    {
        double current = I_s*(exp((nodevoltages[anode]-nodevoltages[cathode])/Vt)-1);
        current = current>10?10:current;
        std::cout << "Current  " <<current << std::endl; 
        return current;
    }
    
    std::vector<double> get_current_derivative(const std::vector<double> &nodevoltages) const override
    {
        std::vector<double> current_derivative(nodevoltages.size(),0);
        current_derivative[anode]= get_current(nodevoltages)/Vt;
        current_derivative[cathode]= -get_current(nodevoltages)/Vt;
        return current_derivative;
    }
};



class Voltage_Component:
    public Component
{

public:
    /*
    given a vector<double> nodevoltages where 
    -each index of nodevoltages corresponds to a node
    -each double of nodevoltages corresponds to a voltage at the given node
    returns the  through the voltage difference across the component from anode to cathode
    */ 
    virtual double get_voltage(const std::vector<double> &nodevoltages) const = 0;

    
};


class Voltage_Source:
    public Voltage_Component
{
private:
    /*
    voltage going from anode to cathode
    */
    double voltage;
public:
    Voltage_Source(int _anode, int _cathode, std:: string _name, double _value){
        anode = _anode;
        cathode = _cathode;
        name = _name;
        value = _value;
    }
    ~Voltage_Source(){}

    double get_voltage(const std::vector<double> &nodevoltages) const override
    {
        return voltage;
    }
    char get_type(){
        return 'V';
    }

    /*
    given an increment computed by "controller" due to a timestep
    updates the value of Voltage_Source:: voltage
    */
    void update_integral(double increment){
        voltage += increment;
    }

};





class Capacitor :
    public Voltage_Component
{
private:
    /*
    integral of current wrt time. Current goes from anode to cathode
    */
    double integral ;
public:
    Capacitor(){}
    ~Capacitor(){}


    double get_voltage(const std::vector<double> &nodevoltages) const override
    {
        return integral/value;
    }

    /*
    given an increment computed by "controller" due to a timestep
    updates the value of Capacitor::integral
    */
    void update_integral(double increment)
    {
        integral += increment;
    }



};




#endif