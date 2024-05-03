#include	<stdio.h>
#include	<pthread.h>

typedef	struct	_course	{
    char*	code;
    char*	name;
} COURSE;

void*	threadFun(void*	arg)	{
    COURSE c = *((COURSE*) arg);
    printf("Welcome	to	%s	- %s\n",	c.code,	c.name);
}
/*	th6.c	cont’d	*/
int	main()	{
    pthread_t	threads[2];
    COURSE	courses[2];
    int	i;
    courses[0].code	= "CMPE382";
    courses[0].name	= "Operating	Systems";
    courses[1].code	= "CMPE361";
    courses[1].name	= "Computer	Organization";
    for	(i	=	0;	i	<	2;	++i)
        pthread_create(&threads[i],	NULL,	threadFun,	&courses[i]);
    for	(i	=	0;	i	<	2;	++i)
        pthread_join(threads[i],	NULL);
    return	0;
}