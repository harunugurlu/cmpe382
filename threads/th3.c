#include	<stdio.h>
#include	<pthread.h>
#include	<unistd.h>
void*	thFun(void	*arg)	{
    int	i	=	0;
    while	(1)	{
        printf("thread	i	=	%d\n",	i++);
        sleep(1);
    }
}
int	main()	{
    pthread_t	thr;
    int	i;
    pthread_create(&thr,	NULL,	thFun,	NULL);
    for	(i = 0; i < 10;	i++)	{
        printf("main	i	=	%d\n",	i);
        if	(i == 5)
            pthread_cancel(thr);
        sleep(1);
    }
    pthread_join(thr, NULL);
    return	0;
}
