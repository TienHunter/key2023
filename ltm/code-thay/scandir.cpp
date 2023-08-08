
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <dirent.h>

const char* rootPath = "/";
char* html = NULL;

int my_filter(const struct dirent * arg)
{
    if (arg->d_type == DT_REG || arg->d_type == DT_DIR)
        return 1;
    else
        return 0;
}

int my_compare(const struct dirent **arg1, const struct dirent ** arg2)
{
    if ((*arg1)->d_type == (*arg2)->d_type) 
        return strcmp((*arg1)->d_name, (*arg2)->d_name);
    else if ((*arg1)->d_type == DT_DIR)
        return -1;
    else
        return 1;
}

void append(char** phtml, const char* str)
{
    char* localhtml = *phtml;
    int oldLen = localhtml == NULL ? 0 : strlen(localhtml);
    int newLen = oldLen + strlen(str) + 1;
    localhtml = (char*)realloc(localhtml, newLen * sizeof(char));
    memset(localhtml + oldLen, 0, (newLen - oldLen) * sizeof(char));
    sprintf(localhtml + oldLen, "%s", str);
    *phtml = localhtml;
}

int main()
{
    struct dirent** result = NULL;
    int n = scandir(rootPath, &result, my_filter, my_compare);
    append(&html,"<html>");
    for (int i = 0;i < n;i++)
    {
        if (result[i]->d_type == DT_DIR)
        {
            append(&html,"<a href = \"");        
            append(&html,result[i]->d_name);
            append(&html,"\"><b>");
            append(&html,result[i]->d_name);
            append(&html,"</b></a><br>\n");
        }else if (result[i]->d_type == DT_REG)
        {
            append(&html,"<a href = \"");        
            append(&html,result[i]->d_name);
            append(&html,"\"><i>");
            append(&html,result[i]->d_name);
            append(&html,"</i></a><br>\n");
        }
        free(result[i]);
        result[i] = NULL;
    }
    append(&html, "</html>");

    FILE* f = fopen("scandir.html", "wt");
    fprintf(f, "%s", html);
    fclose(f);

    free(html);
    html = NULL;
}