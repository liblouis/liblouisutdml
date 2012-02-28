static int
isLineend (int *c)
{
if (c[0] == 10 && c[1] == 13)
return 2;
else if (c[0] == 10 || c[0] == 13)
return 1;
else
return 0;
}

