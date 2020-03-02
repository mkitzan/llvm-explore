// simple functions to optimize with LLVM passes
int min(int v0, int v1)
{
	if (v0 > v1)
	{
		v0 = v1;
	}
	
	return v0;
}

int max(int v0, int v1)
{
	if (v0 < v1)
	{
		v0 = v1;
	}
	
	return v0;
}

int clamp(int value, int m, int M)
{
	value = min(value, M);
	value = max(value, m);
	return value;
}
