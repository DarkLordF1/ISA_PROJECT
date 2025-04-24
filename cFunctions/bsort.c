#include <stdio.h>

void bubble_sort(int arr[], int n){
    int i,j,temp;
    for (i=0 ; i<n-1 ; i++){
        for (j=0 ; j<n-i-1 ; j++){
            if(arr[j]>arr[j+1]){
                temp=arr[j];
                arr[j]=arr[j+1];
                arr[j+1]=temp;
            }
        }
    }
}

// Function to print an array
void printArray(int arr[], int size) {
    int i;
    for (i = 0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

int main(){
    int data[] = {64, 34, 25, 12, 22, 11, 90};
    int n = sizeof(data) / sizeof(data[0]);
    printf("Unsorted array: \n");
    printArray(data, n);
    bubble_sort(data, n);
    printf("Sorted array: \n");
    printArray(data, n);
    return 0;
}