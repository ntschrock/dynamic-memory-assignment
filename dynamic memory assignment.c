#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "fa20_cop3502_as1.h"
#include "leak_detector_c.h"

void remove_crlf(char *s)
{
    char *t = s + strlen(s);
    t--;

    while ((t >= s) && (*t == '\n' || *t == '\r'))
    {
        *t = '\0';
        t--;
    }
}

void get_next_nonblank_line(FILE *ifp, char *s, int max_length)
{
    s[0] = '\0';

    while (s[0] == '\0')
    {
        fgets(s, max_length, ifp);
        remove_crlf(s);
    }
}

int get_num_mon(FILE *ifp) //looks at the input file and grabs the first number and returns it
{
    //variables to get the number
    char num_string[32];
    int num;

    //converts the string to a number and returns it
    get_next_nonblank_line(ifp, num_string, 31);
    num = atoi(num_string);
    return num;
}

monster *new_mon(int nmonsters) //allocates the space for the array of monsters
{
    monster *m = calloc(nmonsters, sizeof(monster));
    return m;
}

void fill_mon(monster *m, char *name, char *element, int population) //allocates and fills in the arrays with data from the input file
{
    //takes the information that was read and fills it into the array of monsters
    m->name = strdup(name);
    m->element = strdup(element);
    m->population = population;
}

void read_mon(FILE *ifp, monster *m) //looks at the input and grabs the next monster
{
    //variable declaration
    int id = 0;
    char mon_info[128];
    char name[32];
    char element[32];
    int population;

    //couldnt use my normal method since it was all one line, take the line in and divide it into its parts that way
    get_next_nonblank_line(ifp, mon_info, 31);
    sscanf(mon_info, "%s %s %d", name, element, &population);

    //calls fill monster with the info taken from the input read
    fill_mon(m, name, element, population);
}

void print_mon(FILE *ofp, monster *m, int total_population, int captures) //function that prints out the monster and info
{
    //calculates the monsters that they capture
    double pop = (double)m->population / (double)total_population;
    int mon_cap = round(pop * captures);
    
    //prints out the monsters name and how many were captured
    if(mon_cap > 0)
    {
        fprintf(ofp, "%d %s\n", mon_cap, m->name);
    }
}

void clr_mon(monster *m) //clears out the monster array
{
    m->id = 0;

    if (m->name != NULL)
    {
        free(m->name);
        m->name = NULL;
    }

    if (m->element != NULL)
    {
        free(m->element);
        m->element = NULL;
    }

    m->population = 0;
}

void dst_mon(monster *m, int nmonsters) //calls the monster clearer for however many monsters there are as determined by get num function and then frees m
{
    for (int i = 0; i < nmonsters; i++)
    {
        clr_mon(m + i);
    }
    free(m);
}

region *new_reg(int nregions) //allocates enough space for each region
{
    region *r = calloc(nregions, sizeof(region));
    return r;
}

int get_num_reg(FILE *ifp)  //gets the number of regions
{
    //variable declaration
    char num_string[128];
    int num;

    //converts the string into a number and returns it
    get_next_nonblank_line(ifp, num_string, 127);
    num = atoi(num_string);
    return num;
}

void fill_reg(region *r, char *name, int nmonsters, int total_population, monster **monsters) //fills in the array of regions
{
    //takes the information from the read function and fills it into the region array
    r->name = strdup(name);
    r->nmonsters = nmonsters;
    r->total_population = total_population;
    r->monsters = monsters;
}

void read_reg(FILE *ifp, region *r, monster *mon_list, int num_mon) //looks at input file and finds all of the information for the region
{
    //variable declaration
    char name[32];
    char nmonsters_string[32];
    int nmonsters;
    int total_population = 0;
    char mon_name[32];

    //gets the name of the region and the number of monsters in the region
    get_next_nonblank_line(ifp, name, 31);
    get_next_nonblank_line(ifp, nmonsters_string, 127);
    nmonsters = atoi(nmonsters_string);

    //allocates space for the double pointer to the monster
    monster** monsters = calloc(nmonsters, sizeof(monster*));
    //function truong helped me come up with. Looks at each monster in the region using the nmonsters for the region, then takes the monsters name, compares each monster in region to the 
    //monster in the monster array and if the name matches then it will take the information and bring it into the region array
    for (int i = 0; i < nmonsters; i++)
    {
        get_next_nonblank_line(ifp, mon_name, 31);
        for (int j = 0; j < num_mon; j++)
        {
            if (strcmp(mon_list[j].name, mon_name) == 0)
            {
                monsters[i] = &mon_list[j];
                total_population += monsters[i]->population;
                break;
            }
        }
    }
    //fills the region array with the information that was taken in the read function
    fill_reg(r, name, nmonsters, total_population, monsters);
}

void print_reg(FILE *ofp, region *r, int captures) //function that prints out the region and info
{
    //outputs the regions name
    fprintf(ofp, "%s\n", r->name);

    //loops for the number of monsters in each region and calls the monster printer
    for(int i = 0; i < r->nmonsters; i++)
    {
        print_mon(ofp, r->monsters[i], r->total_population, captures);
    }
}


void clr_reg(region *r) //sets all of the info in the region array to 0 or null
{
    if (r->name != NULL)
    {
        free(r->name);
        r->name = NULL;
    }

    if (r->monsters != NULL)
    {
        free(r->monsters);
        r->monsters = NULL;
    }

    r->total_population = 0;
}

void dst_reg(region *r, int nregions) //calls the clear and frees the calloc
{
    //clears for the number of regions to get rid of each one
    for (int i = 0; i < nregions; i++)
    {
        clr_reg(r + i);
    }
    free(r);
}


void print_itiny(FILE *ofp, itinerary *itiny) //prints the info in itinerary and sends the rest of print region
{
    //checks the itinerary for how many regions were visited and how many captures they had in each and then sends the print info to print region. 
    for(int i = 0; i < itiny->nregions; i++)
    {
        print_reg(ofp, itiny->regions[i], itiny->captures);
    }
}

//might not be necessary
void clr_itiny(itinerary *itiny) //gets rid of all the stuff stored in itinerary 
{
    itiny->captures = 0;
    
    free(itiny->regions);
    itiny->regions = NULL;
    
    free(itiny);
}

int get_num_train(FILE *ifp) //gets the number of trainers from the input text
{
    //variable declaration
    int num;
    char num_string[128];

    //turns the string into a number and returns it
    get_next_nonblank_line(ifp, num_string, 127);
    num = atoi(num_string);
    return num;
}

trainer *new_train(int ntrainers) //allocates enough space for each trainer
{
    trainer *t = calloc(ntrainers, sizeof(trainer));
    return t;
}

void fill_train(trainer *t, char *name, itinerary *visits) //fills the trainer array with the info taken from the read function 
{
    t->name = strdup(name);
    t->visits = visits;
}

void read_train(FILE *ifp, trainer *t, region *reg_list, int num_reg) //reads the trainer information from the input file
{
    //variable declaration 
    char name[32];
    char captures_string[32];
    int ncaptures = 0;
    char regions_string[32];
    int nregions;
    char reg_name[32];

    //gets the name, amount of captures, and the number regions they visit
    get_next_nonblank_line(ifp, name, 31);
    get_next_nonblank_line(ifp, captures_string, 31);
    ncaptures = atoi(captures_string);
    get_next_nonblank_line(ifp, regions_string, 31);
    nregions = atoi(regions_string);

    //creates enough space for the region double pointer and the visits for itinerary 
    itinerary* visits = calloc(1, sizeof(itinerary));
    region** regions = calloc(nregions, sizeof(region*));
    
    //the visits pointer get the number of regions the trainer visits, the actual regions, and the number of captures they aim to get
    visits->nregions = nregions;
    visits->regions = regions;
    visits->captures = ncaptures;
    
    //same function as before but for the region array, use the same pointer and int but renamed for region
    for(int i = 0; i < nregions; i++)
    {
        get_next_nonblank_line(ifp, reg_name, 31);
        for(int j = 0; j < num_reg; j++)
        {
            if (strcmp(reg_list[j].name, reg_name) == 0)
            {
                visits->regions[i] = &reg_list[j];
                break;
            }
        }
    }
    //fills the trainer with the info gathered in read trainer
    fill_train(t, name, visits);
}

//prints the name of trainer and passes the rest onto itinerary which passes the rest onto reagion 
//which passes the rest onto monster which calculates and outputs the final information
void print_train(FILE *ofp, trainer *t) 
{
    fprintf(ofp, "%s\n", t->name);
    print_itiny(ofp, t->visits);
}

void clr_train(trainer *t) //clears everything stored in the trainer array but also clears the visits which is in the t array but uses the clear itinerary to clear them
{
    if (t->name != NULL)
    {
        free(t->name);
        t->name = NULL;
    }

    if (t->visits != NULL)
    {
        clr_itiny(t->visits);
        t->visits = NULL;
    }
}

void dst_train(trainer *t, int ntrainers) //calls the clearing of the trainer array
{
    for (int i = 0; i < ntrainers; i++)
    {
        clr_train(t + i);
    }
    free(t);
}

void print_space(FILE *ofp) //prints the space in between the trainers and their catch info in the ouput 
{
    fprintf(ofp, "\n");
}

void main(void) //main function, the big chungus 
{
    atexit(report_mem_leak); //calls the memory leak function 
    FILE *ifp; //input file
    FILE *ofp; //output file
    
    //sets the read and write for input and output
    ifp = fopen("cop3502-as1-input.txt", "r");
    ofp = fopen("cop3502-as1-output-schrock-noah.txt", "w+");
    
    int i;

    //all the pointers and number ints for the various structures
    int nmonsters;
    monster *monsters;
    int nregions;
    region *regions;
    int ntrainers;
    trainer *trainers;       

    nmonsters = get_num_mon(ifp); //runs the get num function and assigns returned number to nmonsters
    monsters = new_mon(nmonsters); //creates a new monster for each monster in the num list
    for (i = 0; i < nmonsters; i++) //calls the read monster function based on the number of monsters read in the get num function
    {
        read_mon(ifp, monsters + i);
    }
   
    nregions = get_num_reg(ifp); //runs the get num function and assigns returned number to nregions
    regions = new_reg(nregions); //creates a new region for each monster in the num list
    for (i = 0; i < nregions; i++) //calls the read region  function based on the number of regions read in the get num function
    {
        read_reg(ifp, &regions[i], monsters, nmonsters);
    }
    
    ntrainers = get_num_train(ifp); //runs the get num function and assigns returned number to ntrainers
    trainers = new_train(ntrainers); //creates a new trainer for each monster in the num list
    for(i = 0; i < ntrainers; i++) //calls the read trainer function based on the number of trainers read in the get num function
    {
        read_train(ifp, &trainers[i], regions, nregions);
    }
    //calls the print trainer function for the number of trainers which calls the print itinerary function which calls the print region
    //function which calls the print monster function which calculates and outputs the final information 
    for (i = 0; i < ntrainers; i++)      
    {
        print_train(ofp, &trainers[i]);
        print_space(ofp);
    }

    //closing functions, closes and deletes all the information to prevent any memory leaks 
    dst_mon(monsters, nmonsters);
    dst_reg(regions, nregions);
    dst_train(trainers, ntrainers);
    fclose(ifp);
    fclose(ofp);
}
