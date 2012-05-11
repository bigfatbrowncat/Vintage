bool areEqual(char* s1, char* s2)
{
	char *cur1 = s1, *cur2 = s2;
	for (; *cur1 != 0 && *cur2 != 0; cur1++, cur2++)
	{
		if (*cur1 != *cur2) return false;
	}
	if (*cur1 != *cur2) return false;
	return true;
}
