#include <curl/curl.h>
#include "libxml/HTMLparser.h"
#include "libxml/xpath.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PRODUCTS 16

typedef struct
    {
        char *url;
        char *title;
        char *desc;
    } RecipeProduct;

// Define a struct to hold the response data from libcurl
struct CURLResponse {
    char *html;     // Pointer to store the HTML content
    size_t size;    // Size of the HTML content
};

struct CURLResponse GetRequest(CURL *curl_handle, const char *url);
    
// Callback function to handle received data from libcurl
static size_t WriteHTMLCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;    // Calculate the total size of the received data
    struct CURLResponse *mem = (struct CURLResponse *)userp; // Cast userp to the struct pointer
    
    // Reallocate memory for the HTML content to accommodate the newly received data
    char *ptr = realloc(mem->html, mem->size + realsize + 1);
    if (!ptr) {
        // Check if memory reallocation was successful
        printf("Not enough memory available (realloc returned NULL)\n");
        return 0;
    }
    
    mem->html = ptr;    // Update the pointer to the HTML content
    memcpy(&(mem->html[mem->size]), contents, realsize);   // Copy the received data into the allocated memory
    mem->size += realsize;  // Update the size of the HTML content
    mem->html[mem->size] = 0;   // Add a null terminator at the end of the HTML content
    
    return realsize;    // Return the total size of the received data
}

// Function to perform an HTTP GET request
struct CURLResponse GetRequest(CURL *curl_handle, const char *url) {
    CURLcode res;   // Variable to store the result of libcurl functions
    struct CURLResponse response;   // Struct variable to hold the response data
    
    // Initialize the response struct
    response.html = malloc(1);  // Allocate memory for the HTML content
    response.size = 0;  // Initialize the size of the HTML content
    
    // Specify URL to GET
    curl_easy_setopt(curl_handle, CURLOPT_URL, url); //enter url here
    // Set the write callback function to handle received data
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteHTMLCallback);
    // Pass the response struct to the callback function
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response);
    // Set a User-Agent header for the request
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/117.0.0.0 Safari/537.36");
    
    // Perform the GET request
    res = curl_easy_perform(curl_handle);
    
    // Check for HTTP errors
    if (res != CURLE_OK) {
        fprintf(stderr, "GET request failed: %s\n", curl_easy_strerror(res));
    }
    
    return response;    // Return the response struct containing the HTML content
};

int main(void)
{
        // initialize curl globally
        curl_global_init(CURL_GLOBAL_ALL);
      
        // initialize a CURL instance
        CURL *curl_handle = curl_easy_init();
      
        // initialize the array that will contain
        // the scraped data
        RecipeProduct products[MAX_PRODUCTS];
        int productCount = 0;
      
        // get the HTML document associated with the page
        struct CURLResponse response = GetRequest(curl_handle, "https://www.bbcgoodfood.com/recipes/collection/family-meal-recipes");
    
        // parse the HTML document returned by the server
        htmlDocPtr doc = htmlReadMemory(response.html, (unsigned long)response.size, NULL, NULL, HTML_PARSE_NOERROR);
        xmlXPathContextPtr context = xmlXPathNewContext(doc);
    
        xmlXPathObjectPtr productHTMLElements = xmlXPathEvalExpression((xmlChar *)"//article", context);
       
              // iterate over them and scrape data from each of them
        for (int i = 0; i < productHTMLElements->nodesetval->nodeNr; ++i)
        {
            // get the current element of the loop
            xmlNodePtr productHTMLElement = productHTMLElements->nodesetval->nodeTab[i];

             // set the context to restrict XPath selectors
            // to the children of the current element
            xmlXPathSetContextNode(productHTMLElement, context);

            xmlNodePtr urlHTMLElement = xmlXPathEvalExpression((xmlChar *)".//a", context)->nodesetval->nodeTab[0];
            char *url = (char *)xmlGetProp(urlHTMLElement, (xmlChar *)"href");

            RecipeProduct product;
            product.url = strdup(url);

            free(url);

            // add a new product to the array
            products[productCount] = product;
            productCount++;

             // get the HTML document associated with the page
        //struct CURLResponse response = GetRequest(curl_handle, ("https://www.bbcgoodfood.com/recipes/fajita-style-pasta"));
    
        }

         // free up the allocated resources
        free(response.html);
        xmlXPathFreeContext(context);
        xmlFreeDoc(doc);
        xmlCleanupParser();
      
        // cleanup the curl instance
        curl_easy_cleanup(curl_handle);
        // cleanup the curl resources
        curl_global_cleanup();

           FILE *csvfile = fopen("recipes.csv", "w");
        if (csvfile == NULL){
            perror("Failed to open the csv output file");
        }
        fprintf(csvfile, "url\n");
        for (int i = 0; i < productCount; i++)
        {
            fprintf(csvfile, "%s\n", products[i].url);
        }
      
        // close the CSV file
        fclose(csvfile);
      
        // free the resources associated with each product
        for (int i = 0; i < productCount; i++)
        {
            free(products[i].url);
            
        }
     
        return 0;
    }
