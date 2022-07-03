#include <iostream>
#include <random>
#include <io.h>


using namespace std;

//////////////////////////////////////////////////////////////////////////
const char* FileName = "BigFile";   //название файла
const int maxN = 1000;                  //макс.значение элементов
const int SmallFileSize = 8;            //размер маленького файла в мегабайтах
const int BigFileSize = 1;            //размер большого файла в мегабайтах
//////////////////////////////////////////////////////////////////////////




/*создает файл, заполняет его, закрывает файл*/
int create_big_file(const char* FName) {
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> dist(0, maxN);

    FILE* f;
    errno_t err = fopen_s(&f, FName, "wb+");

    if (f == NULL) {
        cout << "create_big_file: err opening file. code:" << err << endl;
        return -1;
    }
    int BiGFileCount = int(ceil(BigFileSize * 1024 / sizeof(int32_t)));
    for (int i = 0; i < BiGFileCount; i++)
    {
        int32_t a = dist(rng);
        fwrite(&a, sizeof(int32_t), 1, f);
    };
    fclose(f);
    return 0;
}

/*выводит на экран все числа из файла размера BigFile*/
int show_file(const char* fname) {
    FILE* f;
    errno_t err = fopen_s(&f, fname, "rb+");

    if (f == NULL)
        return -1;

    int a;
    while (fread(&a, sizeof(int32_t), 1, f) != NULL) {
        cout << a << " ";
    };
    fclose(f);
    return 0;
};

// сортировка элементов массива в файле
// fname - имя файла
int sort(const char* fname)
{
    // нужно открыть файл и прочитать количество элементов
    FILE* f;
    errno_t err = fopen_s(&f, fname, "rb+");
    if (f == NULL) {
        cout << "sort: err opening file. code:" << err << endl;
        return -1;
    }

    int n;
    n = _filelength(_fileno(f)) / sizeof(int32_t);
    int32_t* tmp = new int32_t[1];
    // пока не равно количеству елементов
    for (int i = 0; i <= n; i++)
    {

        for (int j = 0; j < n - i; j++)
        {

            int  arr;
            int  arrNext;
            fseek(f, sizeof(int32_t) * (j), SEEK_SET); // очередной элемент
            fread(&arr, sizeof(int32_t), 1, f); // считываем очередной элемент
            fseek(f, sizeof(int32_t) * (j + 1), SEEK_SET); // следующий после очередного
            fread(&arrNext, sizeof(int32_t), 1, f); // считываем очередной элемент
            fflush(f);
            if (arr > arrNext)
            {
                // правого, то меняем их местами
                memcpy(tmp, &arr, sizeof(int32_t));
                arr = arrNext;
                arrNext = *tmp;
                fseek(f, sizeof(int32_t) * (j), SEEK_SET); // позиция первого элемента
                fwrite(&arr, sizeof(int), 1, f);

                fseek(f, sizeof(int32_t) * (j + 1), SEEK_SET);
                fwrite(&arrNext, sizeof(int32_t), 1, f);


            };
        };
    };
    fclose(f);
    return 0;
}

int merge_two_files(const char* f1name, const char* f2name, const char* fresname) {
    FILE* f1;
    errno_t errf1 = fopen_s(&f1, f1name, "rb+");

    FILE* f2;
    errno_t errf2 = fopen_s(&f2, f2name, "rb+");

    FILE* fres;
    errno_t errfres = fopen_s(&fres, fresname, "wb+");


    if (f1 == NULL or f2 == NULL or fres == NULL) {
        cout << "create_big_file: err opening file. codes:" << errf1 << ":" << errf2 << ":" << errfres << endl;
        return -1;
    }
    int cnt = 0;
    bool fin = false;
    int32_t n1, n2;
    int f1pos = 0;
    int f2pos = 0;
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
            cout << "merge fin" << endl;
        }

    }
    fclose(f1);
    fclose(f2);
    remove(f1name);
    remove(f2name);
    fclose(fres);
    return 0;
};

int main()
{
    setlocale(LC_ALL, "RUS");
    const char* FileName1 = "File1";
    const char* FileName2 = "File2";
    const char* FileNameRes = "FileRes";

    create_big_file(FileName1);
    create_big_file(FileName2);
    sort(FileName1);
    sort(FileName2);

    merge_two_files(FileName1, FileName2, FileNameRes);
    show_file(FileName2);
    cout << endl << endl;
    show_file(FileNameRes);

   // system("pause");
    return 0;
}