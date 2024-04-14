/*
 * Abitrary shared array fingerprint generation test.
 *
 * By Yunzhou Li
 *
 * Steps:
 *	1. Declare a shared array arr[SIZE], and index i=0.
 *	2. Create 2 threads continously writing data to arr[i], one writing 1, the other writing 0.
 *	3. In main(), increase i and read data of arr[i-1], then add to count[i-1], until i reaches SIZE-1.
 *	4. Reset i=0 and repeat step 3. for several times to get count result.
 *
 * Result:
 *	1. Generated arr shows no pattern.
 *	2. Generated count array shows no pattern.
 *	3. Generated sorted count array shows no pattern.
 *	4.
 */

#include <cstddef>
#include <cstring>
#include <math.h>
#include <pthread.h>
#include <stdio.h>

#define SIZE 4096
#define HIST_SIZE 100

int i = 0;
char arr[SIZE];
int stopped = 0;

void* p_write(void* arg)
{
    int val = *((int*)arg);
    while (!stopped) {
        arr[i] = val;
    }
    return NULL;
}
/*
 * Bubble sort function, descending order.
 * arr - array to sort
 * idx - if not NULL, output sorted index
 * size - array size
 */
void sort(int* arr, int* idx, size_t size)
{
    int i = 0, j = 0;
    if (idx) {
        for (i = 0; i < size; i++)
            idx[i] = i;
    }

    for (i = 0; i < size; i++) {
        for (j = 0; j < size - i - 1; j++) {
            if (arr[j] < arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;

                if (idx) {
                    int idxt = idx[j];
                    idx[j] = idx[j + 1];
                    idx[j + 1] = idxt;
                }
            }
        }
    }
}

void check_seq_diff(const char* filename, int* curr_seq, double threshold)
{
    // Read previous hist (in binary)
    FILE* fp;
    int seq_prev[SIZE];
    fp = fopen(filename, "rb");
    if (fp) {
        fread(seq_prev, sizeof(int), SIZE, fp);
        fclose(fp);
        int match = 0;
        int total = 100;
        int weighted_match = 0;
        int weighted_total = 0;
        int weighted_match2 = 0;
        int weighted_total2 = 0;
        // Histogram difference
        printf("SeqDiff %s\n", filename);
        for (int j = 0; j < 100; ++j) {
            if (seq_prev[j] || curr_seq[j]) {
                weighted_total2 += j;
                weighted_total += j * seq_prev[j];
                if (fabs(seq_prev[j] - curr_seq[j]) / seq_prev[j] <= threshold) {
                    ++match;
                    weighted_match += j * seq_prev[j];
                    weighted_match2 += j;
                }
            }
        }
        printf("Match %d/%d %.2lf%%\n", match, total, (double)match / total * 100.0);
        printf("Weighted match %d/%d %.2lf%%\n", weighted_match, weighted_total, (double)weighted_match / weighted_total * 100.0);
        printf("Weighted match2 %d/%d %.2lf%%\n", weighted_match2, weighted_total2, (double)weighted_match2 / weighted_total2 * 100.0);
    }
}

void write_seq_bin(const char* filename, int* seq, int binary)
{
    FILE* fp;
    if (binary) { // Write in binary
        fp = fopen(filename, "wb");
        fwrite(seq, sizeof(int), SIZE, fp);
        fclose(fp);
    } else { // Write as text
        fp = fopen(filename, "a");
        for (int j = 0; j < 100; ++j) {
            fprintf(fp, "%d ", seq[j]);
        }
        fprintf(fp, "\n");
        fclose(fp);
    }
}

int main(int argc, char** argv)
{
    // Threads and results
    pthread_t threads[2];
    int a = 1, b = 0;
    int count[SIZE];
    int repeat = 10000;

    // The frequency that a `index` length of sequential 1's or 0's array appears
    // E.g.: sequential_count[2]=20000 means 2 sequential 1's or 0's array (11 or 00) appeared 20000 times
    int sequential_count[SIZE];
    int seq_prev[SIZE];
    int cur_seq = 0;
    double threshold = 0.2;
    // Statistics
    int idx[SIZE];
    int prev[SIZE];
    int diff[SIZE];
    int hist[HIST_SIZE];
    int hist_prev[HIST_SIZE];
    int min = -repeat / 10 * 1, max = repeat / 10 * 1;
    int gap = (max - min) / HIST_SIZE;
    int match = 0;
    FILE* fp;

    pthread_create(&threads[0], NULL, p_write, &a);
    pthread_create(&threads[1], NULL, p_write, &b);
    memset(arr, 0, SIZE);
    memset(count, 0, SIZE * sizeof(int));
    memset(prev, 0, SIZE * sizeof(int));
    memset(sequential_count, 0, SIZE * sizeof(int));

    // Generate arr repeatedly and count 1s of each index and sequential arrays
    for (int j = 0; j < repeat; ++j) {
        i = 0;
        char last = 0;
        for (int k = 1; k < SIZE; ++k) {
            i = k;
            count[k - 1] += arr[k - 1];

            if (last != arr[k - 1]) { // if not sequential, add count to result
                ++sequential_count[cur_seq];
                cur_seq = -1;
            }
            ++cur_seq;
            last = arr[k - 1];
        }
        ++sequential_count[cur_seq];
        cur_seq = -1;
    }

    stopped = 1;
    pthread_cancel(threads[0]);
    pthread_cancel(threads[1]);
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    /****************************
     *
     * Matches and histogram check, totally random
     *
     ****************************/
    // Read previous result (in binary)
    // fp=fopen("result.bin", "rb");
    // if(fp){
    // 	fread(prev, sizeof(int), SIZE, fp);
    // 	fclose(fp);
    // 	for(int j=0;j<SIZE;++j){
    // 		// Compare count difference
    // 		diff[j]=count[j]-prev[j];
    // 		if(diff[j]>=min||diff[j]<=max)
    // 			// Generate histogram
    // 			++hist[(diff[j]-min)/gap];
    // 		// Compare match
    // 		if(!diff[j])
    // 			++match;
    // 	}
    // 	printf("Match %d/%d\n",match,SIZE);

    // 	printf("Histogram\n");
    // 	for(int j=0;j<HIST_SIZE;++j){
    // 		printf("%d to %d: %d\n",(min+j*gap),(min+j*gap+gap-1),hist[j]);
    // 	}
    // }

    // // Write current result (in binary)
    // fp=fopen("result.bin", "wb");
    // fwrite(count, sizeof(int), SIZE, fp);
    // fclose(fp);

    /****************************
     *
     * Histogram difference check
     *
     ****************************/
    // Read previous hist (in binary)
    // fp=fopen("hist.bin", "rb");
    // if(fp){
    // 	fread(hist_prev, sizeof(int), HIST_SIZE, fp);
    // 	fclose(fp);

    // 	// Histogram difference
    // 	printf("Histdiff\n");
    // 	for(int j=0;j<100;++j){
    // 		printf("%d to %d: %d\n",(min+j*gap),(min+j*gap+gap-1),hist[j]-hist_prev[j]);
    // 	}
    // }
    // // Write current hist (in binary)
    // fp=fopen("hist.bin", "wb");
    // fwrite(hist, sizeof(int), HIST_SIZE, fp);
    // fclose(fp);

    /****************************
     *
     * Sequence difference check
     *
     ****************************/
    // Read previous sequence (in binary)
    // check_seq_diff("sequence32.bin", sequential_count, threshold);
    // check_seq_diff("sequence1_1.bin", sequential_count, threshold);
    // check_seq_diff("sequence5.bin", sequential_count, threshold);
    // check_seq_diff("sequence10.bin", sequential_count, threshold);
    // check_seq_diff("sequence15.bin", sequential_count, threshold);
    // Write current sequence (in binary)
    write_seq_bin("sequence32.bin", sequential_count, 1);

    // Write current sequence count (append text)
    fp = fopen("sequence.txt", "a");
    for (int j = 0; j < 100; ++j) {
        fprintf(fp, "%d ", sequential_count[j]);
    }
    fprintf(fp, "\n");
    fclose(fp);

    // Write current result (append text)
    // fp=fopen("result.txt", "a");
    // for(int j=0;j<SIZE;++j){
    // 	fprintf(fp,"%d ",count[j]);
    // }
    // fprintf(fp, "\n");
    // fclose(fp);

    /****************************
     *
     * Sorted result check
     *
     ****************************/
    // Sort current result and show top 50
    // sort(count, idx, SIZE);
    // printf("Top 50 current\n");
    // for(int j=0;j<50;++j)
    // 	printf("idx %d val %d\n",idx[j],count[j]);

    // // Sort previous result and show top 50
    // sort(prev, idx, SIZE);
    // printf("\nTop 50 prev\n");
    // for(int j=0;j<50;++j)
    // 	printf("idx %d val %d\n",idx[j],prev[j]);

    // sort(sequential_count, NULL, SIZE);
    // printf("\nTop 50 Sequential counts\n");
    // for(int j=0;j<50;++j)
    // 	printf("%d\n",sequential_count[j]);

    return 0;
}