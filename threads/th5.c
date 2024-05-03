#include	<stdio.h>
#include	<pthread.h>

void*	threadFun(void*	arg)	{
    int	a	=	*((int*)	arg);
    printf("id	=	%d\n",	a);
}

int	main()	{
    pthread_t	threads[50];
    int	i,	ids[50];
    for	(i	=	0;	i	<	50;	++i)	{
        ids[i]	=	i;
        pthread_create(&threads[i],	NULL,	threadFun,	&ids[i]); // Now, no inconsistency. Each thread gets a different memory address.
    }
    for	(i	=	0;	i	<	50;	++i)	{
        pthread_join(threads[i],	NULL);
    }
    return	0;
}
