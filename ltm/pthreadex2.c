/* pthreadex2.c: Minh họa thực hiện tính toán đa luồng. */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/* Các biến toàn cục được chia sẻ giữa các tiến trình. */
int S1 = 0; // Tổng do một luồng tính
int S2 = 0; // Tổng do nhiều luồng tính
int N = 100;
pthread_mutex_t* mutex = NULL; // Khoá truy cập tổng.

void* thread_main(void* arg) {
    int i = *(int*)arg;
    free(arg);

    usleep(10000); // Mô phỏng tính toán 
    /* Yêu cầu độc quyền truy cập tổng, đảm bảo các luồng khác nhau
     * không giẫm đạp lên nhau như sinh viên ở các buổi hoà nhạc. */
    pthread_mutex_lock(mutex);
    /* Các tài nguyên nằm giữa pthread_mutex_lock() và pthread_mutex_unlock()
     * là độc quyền cho luồng này. */
    S2 += i;
    /* Không còn dùng đến tổng nữa. Hãy là người văn minh
     * và cho người khác dùng với. */
    pthread_mutex_unlock(mutex);
    return NULL;
}

int main() {
    mutex = (pthread_mutex_t*)calloc(1, sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);

    /* Lưu lại định danh luồng. */
    pthread_t* tid = (pthread_t*)calloc(N, sizeof(pthread_t));

    /* Sinh ra các luồng tính toán.
     * Ở đây, mỗi luồng nhận tham số là số hạng cộng vào tổng.
     */
    for (int i = 0; i < N; i++) {
        int* arg = (int*)calloc(1, sizeof(int));
        *arg = i;
        pthread_create(&tid[i], NULL, thread_main, (void*)arg);
    }
    /* Luồng chính cần hợp nhất các luồng còn lại 
     * trước khi thoát.
     * `status` chứa giá trị trả về của mỗi luồng
     * ( chính là giá trị trả về từ thread_main() )
     */
    void *status;
    for (int i = 0; i < N; i++) {
        pthread_join(tid[i], &status);
    }

    free(tid);
    tid = NULL;

    pthread_mutex_destroy(mutex);
    free(mutex);
    mutex = NULL;

    for (int i = 0; i < N; i++) {
        S1 += i;
    };
    printf("%d == %d\n", S1, S2);
    return 0;
}
