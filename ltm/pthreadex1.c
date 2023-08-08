/* pthreadex1.c: Minh họa tiến trình đa luồng. */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/* Thủ tục thực thi trong luồng.
 * Vai trò tương tự như main() của luồng chính.
 * Để hỗ trợ truyền vào / trả về các đối tượng có kiểu bất kỳ,
 * các tham số truyền vào / trả về được ép kiểu sang (void*).
 */
void* mythread(void* arg) {
    /* Tham số truyền vào là con trỏ đến một số nguyên.
     * Sao chép lại số nguyên này và giải phóng vùng nhớ gốc.
     */
    int i = *(int*)arg;
    free(arg);

    if (i == 5) {
        printf("Hi from mythread: %d\n", i);
    } else {
        printf("Hello from mythread: %d\n", i);
    }
    return (void*)0;
}

int main()
{
    pthread_t tid;
    for (int i = 0; i < 10; i++) {
        /* Cấp vùng nhớ cho tham số truyền vào luồng.
         * Do các luồng trong tiến trình chia sẻ chung không gian nhớ,
         * mỗi tiến trình cần một vùng tham số riêng để không giẫm đạp lên nhau.
         * Các luồng tự chịu trách nhiệm giải phóng ̣vùng nhớ được giao. */
        int* arg = (int*)calloc(1, sizeof(int));
        *arg = i;
        pthread_create(&tid, NULL, mythread, arg);
    };
    printf("Hello from mainthread\n");
    getchar();
}
