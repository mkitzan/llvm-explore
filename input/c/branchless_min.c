
int branchless_min(int value, int min)
{
	return value ^ ((min ^ value) & -(value > min));
}
