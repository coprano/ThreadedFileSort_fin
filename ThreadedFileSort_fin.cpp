#include <iostream>
#include <random>
#include <io.h>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

//////////////////////////////////////////////////////////////////////////

//это можно поменять, если хочется
const int maxN = 1000;                  //макс.значение элементов
const int SmallFileSize = 1;            //размер маленького файла в Килобайтах
const int BigFileSize = 10;             //размер большого файла в Килобайтах
const int req_num_treads = 8;          //необходимое колво потоков
string FileNameBase = "File";           //Основа для названия файлов
//////////////////////////////////////////////////////////////////////////

//менять только если нужно поменять размеры файлов. Сейчас КБ.
int BiGFileCount = int(ceil(BigFileSize * 1024 / sizeof(int32_t)));
int SmallFileCount = int(ceil(SmallFileSize * 1024 / sizeof(int32_t)));

/////////////////////////////////////////////////////////////////////////

//это не стоит трогать, можно запихнуть в класс, если очень хочется
queue <string> q;//очередь из просто файлов
queue <string> q_sorted;//очередь из сортированных файлов

int thread_count = 0;           //количество потоков
int thread_work = 0;            //количество рабочих потоков

bool q_unlocked = true;
bool q_sorted_locked = false;   //заблокирована ли очередь?
int q_count = 0;                //количество файлов в очереди
int q_sorted_count = 0;         //количество файлов в сортированной очереди

int PartCnt = 0;
/////////////////////////////////////////////////////////////////////////

/// <summary>
/// Создает файл размера BigFileSize [МБ]
/// </summary>
/// <param name="fname">название файла</param>
/// <returns>int 0 -- success; -1 -- fail
/// </returns>
int CreateBigFile(const char* fname) {
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> dist(0, maxN);

    FILE* f;
    errno_t err = fopen_s(&f, fname, "wb+");

    if (f == NULL) {
        cout << "create_big_file: err opening file " << fname << " code:" << err << endl;
        return -1;
    }
    for (int i = 0; i < BiGFileCount; i++)
    {
        int32_t a = dist(rng);
        fwrite(&a, sizeof(int32_t), 1, f);
    };
    fclose(f);
    return 0;
}

/// <summary>
/// Выводит на экран все числа из бинарного файла
/// </summary>
/// <param name="fname">название файла</param>
/// <returns>int 0 -- success; -1 -- fail
/// </returns>
int show_file(const char* fname) {
    FILE* f;
    errno_t err = fopen_s(&f, fname, "rb+");

    if (f == NULL) {
        cout << "show_file: err opening file " << fname << " code:" << err << endl;
        return -1;
    }

    int a;
    while (fread(&a, sizeof(int32_t), 1, f) != NULL) {
        cout << a << " ";
    };
    fclose(f);
    return 0;
};

/// <summary>
/// bubble sort for binary file
/// </summary>
/// <param name="fname">название файла</param>
/// <returns>int 0 -- success; -1 -- fail
/// </returns>
int FileSort(const char* fname)
{
    FILE* f;
    errno_t err = fopen_s(&f, fname, "rb+");
    if (f == NULL) {
        cout << "sort: err opening file " << fname << " code:" << err << endl;
        return -1;
    }
    else{
        int n;
        n = _filelength(_fileno(f)) / sizeof(int32_t);
        int32_t* tmp = new int32_t[1];
        for (int i = 0; i <= n; i++)
        {

            for (int j = 0; j < n - i; j++)
            {
                int  arr;
                int  arrNext;
                fseek(f, sizeof(int32_t) * (j), SEEK_SET);
                fread(&arr, sizeof(int32_t), 1, f);
                fseek(f, sizeof(int32_t) * (j + 1), SEEK_SET);
                fread(&arrNext, sizeof(int32_t), 1, f);
                fflush(f);
                if (arr > arrNext)
                {
                    memcpy(tmp, &arr, sizeof(int32_t));
                    arr = arrNext;
                    arrNext = *tmp;
                    fseek(f, sizeof(int32_t) * (j), SEEK_SET);
                    fwrite(&arr, sizeof(int), 1, f);

                    fseek(f, sizeof(int32_t) * (j + 1), SEEK_SET);
                    fwrite(&arrNext, sizeof(int32_t), 1, f);
              };
            };
        };
        fclose(f);
        return 0;
    }
}

/// <summary>
/// Собирает два отсортированных файла в один
/// </summary>
/// <param name="f1name">название первого маленького файла</param>
/// <param name="f2name">название второго маленького файла</param>
/// <param name="fresname">название итогового файла</param>
/// <returns>int 0 -- success; -1 -- fail
/// </returns>
int merge_two_files(const char* f1name, const char* f2name, const char* fresname) {
    FILE* f1;
    errno_t errf1 = fopen_s(&f1, f1name, "rb+");

    FILE* f2;
    errno_t errf2 = fopen_s(&f2, f2name, "rb+");

    FILE* fres;
    errno_t errfres = fopen_s(&fres, fresname, "wb+");


    if (f1 == NULL or f2 == NULL or fres == NULL) {
        cout << "merge_two_files: err opening files"<< f1name << " " << f2name << " " << fresname << " codes:" << errf1 << ":" << errf2 << ":" << errfres << endl;
        return -1;
    }
    int cnt = 0;
    bool fin = false;
    int32_t n1, n2;
    auto s1 = fread(&n1, sizeof(int32_t), 1, f1);
    auto s2 = fread(&n2, sizeof(int32_t), 1, f2);
    while (!fin) {
        if (s1 != 0 and s2 != 0) {
            if (n1 > n2) {
                fwrite(&n2, sizeof(int32_t), 1, fres);
                s2 = fread(&n2, sizeof(int32_t), 1, f2);
                cnt++;
            }
            else if (n1 < n2) {
                fwrite(&n1, sizeof(int32_t), 1, fres);
                s1 = fread(&n1, sizeof(int32_t), 1, f1);
                cnt++;
            }
            else if (n1 == n2) {
                fwrite(&n1, sizeof(int32_t), 1, fres);
                s1 = fread(&n1, sizeof(int32_t), 1, f1);
                cnt++;
            }
        }
        else if (s1 == 0 and s2 != 0) {
            while (s2 != 0) {
                fwrite(&n2, sizeof(int32_t), 1, fres);
                s2 = fread(&n2, sizeof(int32_t), 1, f2);
                cnt++;
            }
        }
        else if (s1 != 0 and s2 == 0){
            while (s1 != 0) {
                fwrite(&n1, sizeof(int32_t), 1, fres);
                s1 = fread(&n1, sizeof(int32_t), 1, f1);
                cnt++;
            }
        }
        else if (s1 == 0 and s2 == 0) {
            fin = true;
            //cout << "merge fin. proccessed " << cnt << " elements" << endl;
        }

    }
    fclose(f1);
    fclose(f2);
    remove(f1name);
    remove(f2name);
    fclose(fres);
    return 0;
};

/// <summary>
/// Разбивает большой файл на много маленьких
/// </summary>
/// <param name="fname">название большого файла</param>
/// <returns>int 0 -- success; -1 -- fail
/// </returns>
int SplitBigFile(const char* fname) {
    FILE* f;
    errno_t err = fopen_s(&f, fname, "rb+");

    if (f == NULL) {
        cout << "SplitBigFile: err opening file " << fname << " code:" << err << endl;
        return -1;
    }
    
    if (SmallFileSize > BigFileSize) {
        cout << "SplitBigFile err. SmallFileSize > BigFileSize" << endl;
    }

    int fcount = int(ceil((float)BigFileSize / (float)SmallFileSize));
    PartCnt = fcount;
    int32_t n;
    auto s1 = fread(&n, sizeof(int32_t), 1, f);
    while (fcount > 0) {
        FILE* fcurr;
        string fcurr_name = (string)fname + "_part_" + to_string(fcount);
        errno_t err = fopen_s(&fcurr, fcurr_name.c_str(), "wb+");
        if (fcurr == NULL) {
            cout << "sort: err creating file " << fcurr_name << " code:" << err << endl;
            return -1;
        };

        int fcurr_cnt = 0;
        while (s1 != 0 and fcurr_cnt < SmallFileCount) {
            fwrite(&n, sizeof(int32_t), 1, fcurr);
            s1 = fread(&n, sizeof(int32_t), 1, f);
            fcurr_cnt++;
        };
        //cout << fcurr_name << ":wrote " << fcurr_cnt << " entities." << endl;

        fclose(fcurr);
        fcount--;
    };


    fclose(f);
    remove(fname);
    return 0;
}

mutex mtx;
condition_variable cv;

void MultithreadedSorter() {
    while(true){
        if (q_count > 0) {

            unique_lock<mutex> ul(mtx);
            cv.wait(ul, [&] {return q_unlocked; });
            q_unlocked = false;
            if (q_count > 0)
                q_count--;
            else {
                q_unlocked = true;
                ul.unlock();
                cv.notify_all();
                break;
            }
            string s = q.front();
            cout << s << " is being sorted" << endl;
            q.pop();
            this_thread::sleep_for(chrono::milliseconds(1));
            q_unlocked = true;
            ul.unlock();
            cv.notify_one();
            FileSort(s.c_str());
            q_sorted.push(s);
            q_sorted_count++;
            cout <<  s << " is sorted! " << endl;
        }
        else
        {
            cv.notify_all();
            break;
        }
    }
}

void MultithreadedMerge() {
    while (true) {
        if (q_sorted_locked == false and q_sorted_count > 1)
        {

            mtx.lock();
            q_sorted_locked = true;
            this_thread::sleep_for(chrono::milliseconds(10));
            if (q_sorted_count < 2) { q_sorted_locked = false; mtx.unlock(); break; };
            string s1 = q_sorted.front();
            q_sorted.pop();
            string s2 = q_sorted.front();
            q_sorted.pop();
            PartCnt++;
            q_sorted_count--;
            string s3 = (string)FileNameBase + "_part_" + to_string(PartCnt);
            q_sorted_locked = false;
            mtx.unlock();
            merge_two_files(s1.c_str(), s2.c_str(), s3.c_str());
            q_sorted.push(s3.c_str());
        }
        else if (q_sorted_locked == true and q_sorted_count > 1)
        {
            this_thread::sleep_for(chrono::milliseconds(10));
        }
        else if (q_sorted_count == 1)
        {

            break;
        }
    }
};

void remover() {

    for (const auto& entry : fs::directory_iterator("./")) {
        if (entry.path().filename().string().substr(0, ((string)FileNameBase).length() + 6) == (string)FileNameBase + "_part_") {
            remove(entry.path().filename().string().c_str());
        };

    };
};

int main()
{
    setlocale(LC_ALL, "RUS");

    CreateBigFile(FileNameBase.c_str());
    SplitBigFile(FileNameBase.c_str());

    int max_threads = thread::hardware_concurrency();
    int num_threads = min(max_threads, req_num_treads);
    cout << "num)_threrasda:" << num_threads << endl;
    vector<thread> threads(num_threads - 1);



    for (const auto& entry : fs::directory_iterator("./")) {
        if (entry.path().filename().string().substr(0, ((string)FileNameBase).length() + 6) == (string)FileNameBase + "_part_") {
            //cout << entry.path().filename().string() << endl;
            q.push(entry.path().filename().string());
            q_count++;
        };
    }

    //многопоточная сортировка файликов
    for (int i = 0; i < num_threads - 1; i++) {
        cout << "thread " << i << " started" << endl;
        threads[i] = thread(MultithreadedSorter);
    };
    for (int i = 0; i < num_threads - 1; i++) {
        threads[i].join();
    };

    cout << "\n\nEverything is sorted!" << endl << "p:"<<PartCnt << endl;

    //многопоточный merge файликов
    for (int i = 0; i < num_threads - 1; i++) {
        cout << "thread " << i << " started" << endl;
        threads[i] = thread(MultithreadedMerge);
    };

    for (int i = 0; i < num_threads - 1; i++) {
        threads[i].join();
    };

    for (const auto& entry : fs::directory_iterator("./")) {
        if (entry.path().filename().string().substr(0, ((string)FileNameBase).length() + 6) == (string)FileNameBase + "_part_") {
            cout << entry.path().filename().string() << endl;
            show_file(entry.path().filename().string().c_str());
        };

    };
    remover();
    return 0;
}