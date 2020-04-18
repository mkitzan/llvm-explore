int branchless_max(int value, int max)
{
	return value ^ ((max ^ value) & -(value < max));
}
