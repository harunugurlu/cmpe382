#include	<stdio.h>
#include	<unistd.h>
#include	<pthread.h>
void*	threadFun(void*	arg)	{
    printf("Thread	%lu,	pid	=	%d\n",	pthread_self(),	getpid());
}
int	main()	{
    int	i;
    pthread_t	threads[5];
    printf("Main	thread:	%lu,	pid	=	%d\n",	pthread_self(),	getpid());
    for	(i	=	0;	i	<	5;	++i)
        pthread_create(&threads[i],	NULL,	threadFun,	NULL);
    for	(i	=	0;	i	<	5;	++i)
        pthread_join(threads[i],	0);
    return	0;
}