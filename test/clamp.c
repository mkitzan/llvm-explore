// simple functions to optimize with LLVM passes
int min(int value, int min)
{
	if (value > min)
	{
		value = min;
	}
	
	return value;
}

int max(int value, int max)
{
	if (value < max)
	{
		value = max;
	}
	
	return value;
}

int clamp(int value, int m, int M)
{
	value = min(value, M);
	value = max(value, m);
	return value;
}

int branchless_min(int value, int min)
{
	return value ^ ((min ^ value) & -(value > min));
}

int branchless_max(int value, int max)
{
	return value ^ ((max ^ value) & -(value < max));
}
