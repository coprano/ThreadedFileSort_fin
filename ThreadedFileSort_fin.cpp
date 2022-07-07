#include <iostream>
#include <random>
#include <io.h>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <filesystem>
#include <cmath>  
#include <condition_variable>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
//File creation
//70 secs to create 4GB in console
//230 secs to create 4GB in console
//

namespace fs = std::filesystem;
using namespace std;

//////////////////////////////////////////////////////////////////////////

//это можно поменять, если хочется
const int maxN = 1000;                  //макс.значение элементов
unsigned long SmallFileSize = 200;            //размер маленького файла в МегаБайтах
unsigned long BigFileSize = 4096;           //размер большого файла в МегаБайтах
const int req_num_threads = 8;          //необходимое колво потоков
string FileNameBase = "File";           //Основа для названия файлов
//////////////////////////////////////////////////////////////////////////

//менять только если нужно поменять размерности файлов (МБ->КБ и т.д.)
unsigned long long BiGFileCount = (unsigned long long)BigFileSize * 1024 * 1024 / sizeof(int32_t);
unsigned long long SmallFileCount = (unsigned long long)SmallFileSize * 1024 * 1024 / sizeof(int32_t);

/////////////////////////////////////////////////////////////////////////
mutex mtx;
condition_variable cv;

long fcount = (long)ceil((float)BigFileSize / (float)SmallFileSize); //колво маленьких файлов

queue <string> q;//очередь из просто файлов
queue <string> q_sorted;//очередь из сортированных файлов

bool q_unlocked = true;         //разблокирована ли очередь?
bool q_sorted_locked = false;   //заблокирована ли очередь сортированных файлов?
int q_count = 0;                //количество файлов в очереди
int q_sorted_count = 0;         //количество файлов в сортированной очереди

int how_many_working = 0;
int how_many_small_files = 0;//как много файлов отрезано от большого

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
    while (fread(&a, sizeof(int32_t), 1, f) != 0) {
        cout << a << " ";
    };
    fclose(f);
    return 0;
};


int FileSort(const char* fname)
{
    FILE* f;
    errno_t err = fopen_s(&f, fname, "rb");
    if (f == NULL) {
        cout << "sort: err opening file " << fname << " code:" << err << endl;
        return -1;
    }
    else {
        int32_t elem;
        auto s1 = fread(&elem, sizeof(int32_t), 1, f);

        vector <int32_t> vec;
        auto it = vec.begin();

        cout << "imhere!" << endl;
        while (s1 != 0) {
            it = vec.insert(it, elem);
            s1 = fread(&elem, sizeof(int32_t), 1, f);
            it++;
        };
        cout << "vec len:" << vec.size() << endl;
        sort(vec.begin(), vec.end());
        cout << "vec sorted!" << vec.size() << endl;
        fclose(f);
        remove(fname);
        errno_t err = fopen_s(&f, fname, "wb");
        if (f == NULL) {
            cout << "show_file: err opening file " << fname << " code:" << err << endl;
            return -1;
        }

        for (it = vec.begin(); it != vec.end(); it++) {
            fwrite(&*it, sizeof(int32_t), 1, f);
        }

        fclose(f);
        return 0;
    }
}
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

int SplitBigFile_multithreaded(string fname) {
    FILE* f;
    errno_t err = fopen_s(&f, fname.c_str(), "rb");

    if (f == NULL) {
        cout << "SplitBigFile: err opening file " << fname << " code:" << err << endl;
        return -1;
    }

    if (SmallFileSize > BigFileSize) {
        cout << "SplitBigFile err. SmallFileSize > BigFileSize" << endl;
    }
    bool is_changeable = true;
    
    while (how_many_small_files < fcount) {
        unique_lock <mutex> ul(mtx);
        cv.wait(ul, [&] {return is_changeable; });
        is_changeable = false;
        if (how_many_small_files == fcount)
        {
            is_changeable = false;
            ul.unlock();
            cv.notify_one();
            break;
        };

        int32_t n;
        string fcurr_name = (string)fname + "_part_" + to_string(how_many_small_files);

        FILE* fcurr;
        errno_t err1 = fopen_s(&fcurr, fcurr_name.c_str(), "wb+");
        if (fcurr == NULL) {
            cout << "sort: err creating file " << fcurr_name << " code:" << err << endl;
            return -1;
        };

        unsigned long long offset = sizeof(int32_t) * (how_many_small_files)*SmallFileCount;
        _fseeki64(f, offset, SEEK_SET);
        cout << how_many_small_files << "; starting pos" << offset << ":real:"<< _ftelli64(f) << endl;
        how_many_small_files++;
        is_changeable = true;
        ul.unlock();
        cv.notify_one();
        long fcurr_cnt = 1;
        auto s1 = fread(&n, sizeof(int32_t), 1, f);


        while (s1 != 0 and fcurr_cnt < SmallFileCount) {
            fwrite(&n, sizeof(int32_t), 1, fcurr);
            s1 = fread(&n, sizeof(int32_t), 1, f);
            fcurr_cnt++;
        };
        cout << fcurr_name << ":WROTE ENTITIES:" << fcurr_cnt <<" " << _ftelli64(f) << endl;;


        fclose(fcurr);
    }

    fclose(f);
    //remove(fname.c_str());
    return 0;
}



void MultithreadedSorter() {
    while(true){
        unique_lock<mutex> ul(mtx);
        cv.wait(ul, [&] {return q_unlocked; });
        q_unlocked = false;
        if (q.size() == 0) {
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
        cv.notify_one();
        //!q_sorted_count++;
        cout <<  s << " is sorted! " << endl;
    }
}


void MultithreadedMerge() {
    while (true) {
        condition_variable cv;
        unique_lock <mutex> ul(mtx);
        cv.wait(ul, [&] {return true; });//false чтобы стоп
        //cv.wait(ul, [&] {return (how_many_working == 0) or (q_sorted.size() > 1 and q_sorted.size() - how_many_working > 1) or (q_sorted.size() == 1 and how_many_working == 0); });//false чтобы стоп
        if (q_sorted.size() == 1 and how_many_working == 0) { /*cout << "thread fin!" << endl;*/ ul.unlock(); cv.notify_all(); break; }//1 файл и все остановлены
        else if (q_sorted.size() > 1 and q_sorted.size() - how_many_working > 1) { //если в очереди 2 файла и можно ухватить
            q_sorted_count--;
            how_many_working++;
            string s1 = q_sorted.front();
            q_sorted.pop();
            string s2 = q_sorted.front();
            q_sorted.pop();
            fcount++;
            string s3 = (string)FileNameBase + "_part_" + to_string(fcount);
            ul.unlock();
            cv.notify_one();
            merge_two_files(s1.c_str(), s2.c_str(), s3.c_str());
            q_sorted.push(s3.c_str());
            q_sorted_count++;
            how_many_working--;
        }
        else {}
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
    remover();
    cout << "begin " << SmallFileCount << endl;
    int max_threads = thread::hardware_concurrency();
    int num_threads = min(max_threads, req_num_threads);


    auto start = clock();



    CreateBigFile(FileNameBase.c_str());
    vector<thread> threads(num_threads - 1);

    for (int i = 0; i < num_threads - 1; i++) {
        cout << "thread " << i << " started" << endl;
        threads[i] = thread(SplitBigFile_multithreaded, FileNameBase.c_str());
    };
    for (int i = 0; i < num_threads - 1; i++) {
        threads[i].join();
    };




    for (const auto& entry : fs::directory_iterator("./")) {
        if (entry.path().filename().string().substr(0, ((string)FileNameBase).length() + 6) == (string)FileNameBase + "_part_") {
            //cout << entry.path().filename().string() << endl;
            q.push(entry.path().filename().string());
            q_count++;
        };
    }

    for (int i = 0; i < num_threads - 1; i++) {
        cout << "thread " << i << " started" << endl;
        threads[i] = thread(MultithreadedSorter);
    };
    for (int i = 0; i < num_threads - 1; i++) {
        threads[i].join();
    };

    for (int i = 0; i < num_threads - 1; i++) {
        cout << "thread " << i << " started" << endl;
        threads[i] = thread(MultithreadedMerge);
    };
    for (int i = 0; i < num_threads - 1; i++) {
        threads[i].join();
    };

    double t = (double)(clock() - start) / CLOCKS_PER_SEC;
    cout << "\nTime taken (seconds):\n\t\n\n" << t;

    system("pause");


    for (const auto& entry : fs::directory_iterator("./")) {
        if (entry.path().filename().string().substr(0, ((string)FileNameBase).length() + 6) == (string)FileNameBase + "_part_") {
            //cout << entry.path().filename().string() << endl;
            show_file(entry.path().filename().string().c_str());
        };
    };


    return 0;
}