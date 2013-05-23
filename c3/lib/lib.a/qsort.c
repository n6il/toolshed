static int	size;
static int	(*comp)();

qsort(base, nel, width, compar)
char	*base;
int		(*compar)();
{
	size = width;
	comp = compar;
	sort(base, base + ((nel - 1) * size));
}

static
sort(l, r)
char	*l, *r;
{
	register char	*i,*j;
	char			*x;

	while(l < r) {
		i = l;
		j = r;
		x = l + (((r - l)/(size * 2)) * size);

		do {
			while ((*comp)(i, x) < 0)
				i += size;
			while ((*comp)(x, j) < 0)
				j -= size;
			if (i <= j) {
				if (i < j) {
					swap(i, j);
					if (x == i)
						x = j;
					else if(x == j)
						x = i;
				}
				i += size;
				j -= size;
			}
		} while (i <= j);

		if ((int)(j - l) > (int)(r - i)) {
			sort(i, r);
			r = j;
		} else {
			sort(l, j);
			l = i;
		}
	}
}

static
swap(i, j)
register char	*i, *j;
{
	int		count;
	char	temp;

	for (count = size; count--;) {
		temp = *i;
		*i++ = *j;
		*j++ = temp;
	}
}
