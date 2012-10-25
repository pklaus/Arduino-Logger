
// Display mode
mode(0);

// Display warning for floating point exception
ieee(1);


// Properties of the NTC:
B = 3625;R0 = 10000;T0 = 25+273;

// Properties of the measurement circuit:
Rref = 500;Uvcc = 4.6;ADCbits = 10;

// Map the measurement range of the ADC:
Um = 0:Uvcc/(2^ADCbits-1):Uvcc;
// remove the first and last element:
Um = Um(2:$-1);

// Calculate the distinguishable resistance values of the NTC:
Rt = Rref*(Uvcc ./Um-1);// by measuring the voltage accross the ref res.
//Rt = Rref ./ (Uvcc ./ Um - 1); % measuring the voltage across the NTC

// Calculate the temperature values belonging to those NTC resistances:
T = 1 ./(log(Rt/R0)/B+1/T0)-273.15;

// Plot the temperatures versus their differences
// !! L.21: Matlab function scatter not yet converted, original calling sequence used.
plot2d(T(1:$-1),diff(T,1,firstnonsingleton(T)),-1);

// Plot the Temperatures vs. discrete ADC values
//plot2d([1:2^ADCbits-2],T,-1)
