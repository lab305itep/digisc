double PMTYAverageLightColl(double *y, double *par)
{
//<func(x)=1>
	const double FuncAverage = 1.00147;
	double rez, x;

	x = y[0] - 50;
	rez = 0.987387*exp(-0.0016*x) + 0.023973*exp(-0.0877*x) 
		- 0.0113581*exp(-0.1042*x) - 2.30972E-6*exp(0.2214*x);
	rez /= FuncAverage;
	return rez;
}

double SiPMYAverageLightColl(double *y, double *par)
{
//<func(x)=1>
	const double FuncAverage = 1.02208;
	double rez, x;

	x = y[0] - 50;
	rez = 0.00577381*exp(-0.1823*x) + 0.999583*exp(-0.0024*x)
		- 8.095E-13*exp(0.5205*x) - 0.00535714*exp(-0.1838*x);
	rez /= FuncAverage;
	return rez;
}
