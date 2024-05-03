#include	<stdio.h>
#include	<pthread.h>
void*	threadFun(void*	arg)	{
    int	a	=	*((int*)	arg);
    printf("i	=	%d\n",	a);
}
int	main()	{
    pthread_t	threads[50];
    int	i;
    for	(i	=	0;	i	<	50;	++i)	{
        pthread_create(&threads[i],	NULL,	threadFun,	&i); // inconsistency. all	threads	access	and	operate	on	(read	and	write)	the	same	address
    }
    /*	th4.c	cont’d	*/
    for	(i	=	0;	i	<	50;	++i)	{
        pthread_join(threads[i],	NULL);
    }
    return	0;
}