#include <jni.h>
#include <sys/wait.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/file.h>


void create_file_if_not_exist(char *path) {
    FILE *fp = fopen(path, "ab+");
    if (fp) {
        fclose(fp);
    }
}

void set_process_name(JNIEnv *env) {
    jclass process = env->FindClass("android/os/Process");
    jmethodID setArgV0 = env->GetStaticMethodID(process, "setArgV0", "(Ljava/lang/String;)V");
    jstring name = env->NewStringUTF("app_d");
    env->CallStaticVoidMethod(process, setArgV0, name);
}

int lock_file(char *lock_file_path) {
    int lockFileDescriptor = open(lock_file_path, O_RDONLY);
    if (lockFileDescriptor == -1) {
        lockFileDescriptor = open(lock_file_path, O_CREAT, S_IRUSR);
    }
    int lockRet = flock(lockFileDescriptor, LOCK_EX);
    if (lockRet == -1) {
        return 0;
    } else {
        return 1;
    }
}


void notify_and_waitfor(char *observer_self_path, char *observer_daemon_path) {
    int observer_self_descriptor = open(observer_self_path, O_RDONLY);
    if (observer_self_descriptor == -1) {
        observer_self_descriptor = open(observer_self_path, O_CREAT, S_IRUSR | S_IWUSR);
    }
    int observer_daemon_descriptor = open(observer_daemon_path, O_RDONLY);
    while (observer_daemon_descriptor == -1) {
        usleep(1000);
        observer_daemon_descriptor = open(observer_daemon_path, O_RDONLY);
    }
    remove(observer_daemon_path);
}

void do_daemon(JNIEnv *env, jobject jobj, char *indicator_self_path, char *indicator_daemon_path,
               char *observer_self_path, char *observer_daemon_path, int code,
               const uint8_t *data, size_t data_size) {
    int lock_status = 0;
    int try_time = 0;
    while (try_time < 3 && !(lock_status = lock_file(indicator_self_path))) {
        try_time++;
        usleep(10000);
    }
    if (!lock_status) {
        return;
    }

    notify_and_waitfor(observer_self_path, observer_daemon_path);

    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder = sm->getService(String16("activity"));
    Parcel parcel;
    parcel.setData(data, data_size);

    int pid = getpid();

    lock_status = lock_file(indicator_daemon_path);
    if (lock_status) {
        int result = binder.get()->transact(code, parcel, NULL, 0);
        remove(observer_self_path);// it`s important ! to prevent from deadlock
//        java_callback(env, jobj, DAEMON_CALLBACK_NAME);
        if (pid > 0) {
            killpg(pid, SIGTERM);
        }
    }
}



//com.alive.demo.keeplive
extern "C" JNIEXPORT void JNICALL
Java_com_alive_demo_keeplive_NativeKeepAlive_doDaemon(JNIEnv *env, jobject jobj,
                                                     jstring indicatorSelfPath,
                                                     jstring indicatorDaemonPath,
                                                     jstring observerSelfPath,
                                                     jstring observerDaemonPath,
                                                     jint code, jint parcel_ptr) {
    if (indicatorSelfPath == NULL || indicatorDaemonPath == NULL || observerSelfPath == NULL ||
        observerDaemonPath == NULL) {
        return;
    }
    if (parcel_ptr == 0) {
        return;
    }

    char *indicator_self_path = (char *) env->GetStringUTFChars(indicatorSelfPath, 0);
    char *indicator_daemon_path = (char *) env->GetStringUTFChars(indicatorDaemonPath, 0);
    char *observer_self_path = (char *) env->GetStringUTFChars(observerSelfPath, 0);
    char *observer_daemon_path = (char *) env->GetStringUTFChars(observerDaemonPath, 0);

    //Parcel *parcel = (Parcel *) parcel_ptr;
    //size_t data_size = parcel->dataSize();
    size_t data_size = parcel_ptr;
    int fd[2];
    if (pipe(fd) < 0) {
        return;
    }

    pid_t pid;
    if ((pid = fork()) < 0) {
        exit(-1);
    } else if (pid == 0) { //第一个子进程
        if ((pid = fork()) < 0) {
            exit(-1);
        } else if (pid > 0) {
            // 托孤
            exit(0);
        }

        close(fd[1]);
        uint8_t data[data_size];
        int result = read(fd[0], data, data_size);
        close(fd[0]);


        const int MAX_PATH = 256;
        char indicator_self_path_child[MAX_PATH];
        char indicator_daemon_path_child[MAX_PATH];
        char observer_self_path_child[MAX_PATH];
        char observer_daemon_path_child[MAX_PATH];

        strcpy(indicator_self_path_child, indicator_self_path);
        strcat(indicator_self_path_child, "-c");

        strcpy(indicator_daemon_path_child, indicator_daemon_path);
        strcat(indicator_daemon_path_child, "-c");

        strcpy(observer_self_path_child, observer_self_path);
        strcat(observer_self_path_child, "-c");

        strcpy(observer_daemon_path_child, observer_daemon_path);
        strcat(observer_daemon_path_child, "-c");

        create_file_if_not_exist(indicator_self_path_child);
        create_file_if_not_exist(indicator_daemon_path_child);

        set_process_name(env);

        // 直接传递parcel，会导致监听不到进程被杀；改成传输u8*数据解决了
        do_daemon(env, jobj, indicator_self_path_child, indicator_daemon_path_child,
                  observer_self_path_child, observer_daemon_path_child, code, data, data_size);
    }

    close(fd[0]);
    uint8_t datas[data_size];
    //int result = write(fd[1], parcel->data(), data_size);
    int result = write(fd[1], datas, data_size);
    close(fd[1]);

//    if (waitpid(pid, NULL, 0) != pid)

    do_daemon(env, jobj, indicator_self_path, indicator_daemon_path, observer_self_path,
              observer_daemon_path, code, datas, data_size);
}