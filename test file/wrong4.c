int process(int a,int b,int c)
{
	int i;
	int j;
	i=0; 	
	if(a>(b-c))
	{
		j=a+(b*c+1);
	}
	else
	{
		j=a;
	}
	while(i<=100)
	{
		i=j*2;
		j=i;
	}
	return i;
}

int subfunc(int a)
{
	a=a+6;
	return a*2;
}

void main(void)
{
	int a;
	int b;
	int c;
	a=3;
	b=4;
	c=2;
	a=p(a,b,subfunc(c));
	return;
}
#