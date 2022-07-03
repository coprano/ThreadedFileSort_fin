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


//const int BiGFileCount = int(ceil(BigFileSize * 1024 * 1024 / sizeof(int32_t)));
//const int SmallFileCount = int(ceil(SmallFileSize * 1024 * 1024 / sizeof(int32_t)));


/*создает файл, заполняет его, закрывает файл*/
int create_big_file(const char* FileName) {
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> dist(0, maxN);

    FILE* f;
    errno_t err = fopen_s(&f, FileName, "wb+");

    if (f == NULL)
        return -1;
    //int BiGFileCount = int(ceil(BigFileSize * 1024 * 1024 / sizeof(int32_t)));
    int BiGFileCount = int(ceil(BigFileSize * 1024 * 1024 / sizeof(int32_t)));
    for (int i = 0; i < BiGFileCount; i++)
    {
        int32_t a = dist(rng);
        fwrite(&a, sizeof(int32_t), 1, f);
    };
    cout << "BFC: " << BiGFileCount << endl;
    fclose(f);
    return 0;
}

/*выводит на экран все числа из файла размера BigFile*/
int show_file(const char* fname) {
    FILE* f;
    errno_t err = fopen_s(&f, FileName, "rb+");

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
    if (f == NULL)
        return -1;

    int n;
    n = _filelength(_fileno(f)) / sizeof(int32_t);
    cout << endl << " N: " << n << endl << endl;
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


    if (f1 == NULL or f2 == NULL or fres == NULL)
        return -1;
    bool fin = false;
    int n1, n2;
    while (fread(&n1, sizeof(int32_t), 1, f1) != NULL or fread(&n2, sizeof(int32_t), 1, f2) != NULL) {
        //если в обоих файлах есть числа
        if (fread(&n1, sizeof(int32_t), 1, f1) != NULL and fread(&n2, sizeof(int32_t), 1, f2) != NULL) {
            if (n1 > n2) {
                fwrite(&n2, sizeof(int32_t), 1, fres);
                fseek(f2, sizeof(int32_t), SEEK_CUR);
                cout << "case1_1" << endl;
            }
            else if (n1 < n2) {
                fwrite(&n1, sizeof(int32_t), 1, fres);
                fseek(f1, sizeof(int32_t), SEEK_CUR);
                cout << "case1_2" << endl;
            }
            else if (n1 == n2) {
                fwrite(&n1, sizeof(int32_t), 1, fres);
                fseek(f1, sizeof(int32_t), SEEK_CUR);
                cout << "case1_3" << endl;
            }
            else cout << "err in merge_two_files idk why" << endl;
        }
        else if (fread(&n1, sizeof(int32_t), 1, f1) == NULL and fread(&n2, sizeof(int32_t), 1, f2) != NULL) {
            while (fread(&n2, sizeof(int32_t), 1, f2) != NULL) {
                fwrite(&n2, sizeof(int32_t), 1, fres);
                fseek(f2, sizeof(int32_t), SEEK_CUR);
                fflush(f1);
                cout << "case2 n1:n2 " << n1 << " " << n2 << endl;
            }
        }
        else if (fread(&n2, sizeof(int32_t), 1, f2) == NULL and fread(&n1, sizeof(int32_t), 1, f1) != NULL) {
            while (fread(&n1, sizeof(int32_t), 1, f1) != NULL) {
                fwrite(&n1, sizeof(int32_t), 1, fres);
                fseek(f1, sizeof(int32_t), SEEK_CUR);
                fflush(f2);
                cout << "case3 n1:n2 " << n1 << " " << n2 << endl;
            }
        }
        if (fread(&n2, sizeof(int32_t), 1, f2) == NULL and fread(&n1, sizeof(int32_t), 1, f1) == NULL) {
            fin = true;
            cout << "fin" << endl;
        }
    }


    remove(f1name);
    remove(f2name);
    fclose(fres);
    return 0;
};

int main()
{
    cout << (false or false) << endl;
    const char* FileName1 = "File1MB";
    const char* FileName2 = "File2MB";
    const char* FileNameRes = "FileRes";
    setlocale(LC_ALL, "RUS");
    create_big_file(FileName1);
    //create_big_file(FileName2);
    sort(FileName1);
    sort(FileName2);
    merge_two_files(FileName1, FileName2, FileNameRes);
    show_file(FileNameRes);

    system("pause");
    return 0;
}