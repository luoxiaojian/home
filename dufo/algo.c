#include <stdio.h>
#include <stdlib.h>

#define MIN(a, b) ((a)>(b)?(b):(a))
#define MAX(b, a) ((a)>(b)?(b):(a))

int *period, *execute;
int true_task_num, task_num, processor_num, lcm_period;
FILE *flog, *fin;

int *total_to_exe, *cur_to_exe, *laxity;
int **execute_flag;
int *zone_flag, *time_to_zone;

int row_num, *rows, *b;

void initial() {
	int i, j, *time, pre, cur;

	fscanf(fin, "%d%d%d%d", &true_task_num, &task_num, &processor_num, &lcm_period);

	period=(int *)malloc(sizeof(int)*task_num);
	execute=(int *)malloc(sizeof(int)*task_num);
	total_to_exe=(int *)malloc(sizeof(int)*task_num);
	cur_to_exe=(int *)malloc(sizeof(int)*task_num);
	laxity=(int *)malloc(sizeof(int)*task_num);
	execute_flag=(int **)malloc(sizeof(int *)*task_num);
	for(int i=0; i<task_num; i++)
		execute_flag[i]=(int *)malloc(sizeof(int *)*lcm_period);

	time_to_zone=(int*)malloc(sizeof(int)*lcm_period);

	for(i=0; i<task_num; i++)
		fscanf(fin, "%d", &period[i]);
	for(i=0; i<task_num; i++)
		fscanf(fin, "%d". &execute[i]);

	for(i=0; i<task_num; i++)
		for(j=0; j<lcm_period; j++)
			execute_flag[i][j]=0;

	time=(int*)malloc(sizeof(int)*lcm_period);
	for(i=0; i<lcm_period; i++)
		time[i]=0;
	for(i=0; i<task_num; i++)
		for(j=0; j<lcm_period/period[i]; j++)
			time[j*period[i]]=1;
	row_num=0;
	for(i=0; i<lcm_period; i++)
		row_num+=time[i];
	rows=(int*)malloc(sizeof(int)*row_num);
	b=(int*)malloc(sizeof(int)*(row_num+1));
	j=row_num-1;
	pre=lcm_period;
	for(i=lcm_period-1; i>=0; i--) {
		if(time[i]==1) {
			rows[j]=pre-i;
			j--;
			pre=i;
		}
	}
	free(time);
	b[0]=0;
	for(i=0; i<row_num; i++)
		b[i+1]=b[i]+rows[i];

	zone_flag=(int*)malloc(sizeof(int)*row_num);
	cur=0;
	for(i=0; i<lcm_period; i++) {
		if(i==b[cur+1])
			cur++;
		time_to_zone[i]=cur;
	}
}

int more_src_exchgbl(int i, int cur, int next) {
	return ((laxity[i]>0) && (total_to_exe[i]<execute[i]));
}

int more_dst_exchgbl(int i, int cur, int next) {
	return ((cur_to_exe[i]>0) && ((next-cur-cur_to_exe[i])<(period[i]-execute[i])));	
}

int less_src_exchgbl(int i, int cur, int next) {
	return ((cur_to_exe[i]<(next-cur)) && (cur_to_exe[i]<execute[i]));
}

int less_dst_exchgbl(int i, int cur, int next) {
	return ((total_to_exe[i]>0) && (laxity[i]<(period[i]-execute[i])));
}

void clear_zone_flag() {
	int i;
	for(i=0; i<row_num; i++) {
		zone_flag[i]=0;	
	}
}

int search_src_zone(int task_set_src[], int src_num, int cur, int next, src_exchgbl_func src_exchgbl, int *src) {
	int i, j;
	int s;
	int arrival, deadline;

	if(src_num<=0) {
		printf("number of source job error!\n");
		exit(0);
	}

	for(i=0; i<src_num; i++) {
		s=task_set_src[i];
		if(!src_exchgbl(s, cur, next)) {
			continue;
		}

		arrival=JOB_RELEASE_TIME(s, cur);
		deadline=MIN(JOB_DEADLINE_TIME(s, cur), cur);

		for(j=arrival; j<deadline; j++) {
			if(zone_flag[time_to_zone[j]]!=0) {
				j=b[time_to_zone[j]+1]-1;
				continue;
			}	
			if(execute_flag[s][j]==0) 
				continue;

			*src=s;
			return time_to_zone[j];
		}
	}

	*src=-1;
	return -1;
}

int search_zone_dst(int task_set_dst[], int dst_num, int cur, int next, dst_exchgbl_func dst_exchgbl, int zone) {
	int i, j;
	int d;

	if(dst_num<=0) {
		printf("number of destination job error!\n");
		exit(0);
	}
	
	int zone_begin, zone_end;
	zone_begin=b[zone];
	zone_end=b[zone+1];

	for(i=0; i<dst_num; i++) {
		d=task_set_dst[i];

		if((!dst_exchgbl(d, cur, next)) || (JOB_RELEASE_TIME(d, zone_begin)!=JOB_RELEASE_TIME(d, cur))) {
			continue;
		}

		for(j=zone_begin; j<zone_end; j++) {
			if(execute_flag[d][j]==0) {
				return d;
			}	
		}
	}
	
	return -1;
}

int exchange_execution_zone_directly(int cur, int next, int cur_capacity, src_exchgbl_func src_exchgbl, dst_exchgbl_func dst_exchgbl, exchg_prcs_func exchg_prcs) {
	int i, j, src, dst;
	int src_zone;
	int s, d;

	for(i=0, src=0, dst=0; i<task_num; i++) {
		if(src_exchgbl(i, cur, next)) {
			task_set_src[src++]=i;
		}
		if(dst_exchgbl(i, cur, next)) {
			task_set_dst[dst++]=i;
		}
	}

	clear_zone_flag();

	while(cur_capacity!=0) {
		if((src_zone=search_src_zone(task_set_src, src, cur, next, src_exchgbl, &s))<0) {
			return cur_capacity;
		}
		if((d=search_zone_dst(task_set_dst, dst, cur, next, dst_exchgbl, src_zone))<0) {
			zone_flag[src_zone]=1;
			continue;
		}

		int zone_begin;
		int zone_end;

		zone_begin=b[src_zone];
		zone_end=b[src_zone+1];
		
		_exchange_execution_zone(s, d, zone_begin, zone_end);
		cur_capacity=exchg_prcs(s, d, cur_capacity);
	}
	
	return cur_capacity;
}

void schedule_laxity_hierarchy() {
	int i, j;
	int cur, next;
	int cur_capacity;
	int window;
	int zind;

	int *least_to_exe, *most_to_exe;
	least_to_exe=(int*)malloc(sizeof(int)*task_num);
	most_to_exe=(int*)malloc(sizeof(int)*task_num);

	for(i=0; i<task_num; i++) {
		total_to_exe[i]=0;
		least_to_exe[i]=0;
		most_to_exe[i]=0;
		cur_to_exe[i]=0;
		laxity[i]=0;
	}

	for(i=0; i<task_num; i++) {
		for(j=0; j<lcm_period; j++) {
			execute_flag[i][j]=0;
		}
	}

	for(zind=0; zind<row_num; zind++) {
		cur=b[zind];
		next=b[zind+1];
		window=next-cur;

		cur_capacity=window*processor_num;

		for(i=0; i<task_num; i++) {
			if(!(cur%period[i])) {
				if(total_to_exe[i]>0)
					printf("Deadline miss!!!\n");
				return ;
			} else {
				total_to_exe[i]=execute[i];
				laxity[i]=period[i]-execute[i];
			}
		}

		for(i=0; i<task_num; i++) {
			least_to_exe[i]=MAX(0, window-laxity[i]);
			most_to_exe[i]=MIN(total_to_exe[i], window);
			laxity[i]-=window;
		}

		for(i=0; i<task_num; i++) {
			cur_to_exe[i]=least_to_exe[i];
			total_to_exe[i]-=least_to_exe[i];
			laxity[i]+=least_to_exe[i];
			cur_capacity-=least_to_exe[i];
		}

		if(cur_capacity<0) {
			cur_capacity=exchange_execution_zone_directly(cur, next, cur_capacity, more_src_exchgbl, more_dst_exchgbl, more_exchg_prcs);
		}

		if(cur_capacity<0) {
			cur_capacity=exchange_execution_zone_indirectly(cur, next, cur_capacity, more_src_exchgbl, more_dst_exchgbl, more_exchg_prcs);
		}

		if(cur_capacity<0) {
			printf("capacity not enough!!!\n");
			return ;
		}

		cur_capacity=assign_cur_execution(cur, pre, cur_capacity, least_to_exe, most_to_exe);

		if(cur_capacity>0) {
			cur_capacity=exchange_execution_zone_directly(cur, next, cur_capacity, less_src_exchgbl, less_dst_exchgbl, less_exchg_prcs);
		}

		if(cur_capacity>0) {
			cur_capacity=exchange_execution_zone_indirectly(cur, next, cur_capacity, less_src_exchgbl, less_dst_exchgbl, less_exchg_prcs);
		}

		if(cur_capacity>0) {
			printf("capacity to mush!!!\n");
			return ;
		}
		
	}
}

void schedule_task_set() {
	schedule_laxity_hierarchy();
	check_scheduling();
}

int main(int argc, char **argv)
{
	if(argc<3) {
		printf("usage: ./algo <ts_filename> <log_filename>\n");
		return 0;
	}

	fin=fopen(argv[1], "r");
	flog=fopen(argv[2], "w");

	initial();

	schedule_task_set();

}
