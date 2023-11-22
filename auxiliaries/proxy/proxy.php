<?php

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: GET, POST, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type");
header("Content-Type: application/json; charset=utf-8");
header('Cache-Control: public, max-age=30');

doTheProxy();

function doTheProxy()
{
    // Get the original path and query parameters from the current URL
    $requestUri = $_SERVER['REQUEST_URI'];
    $requestUri = substr($requestUri, strrpos($requestUri, "/api/v3"));

    $expireTime = 10 * 60;
    if (str_contains($requestUri, "/market_chart?")) {
        if (str_contains($requestUri, "&days=60")) {
            $expireTime = 6 * 60 * 60;
        } else {
            $expireTime = 30 * 60;
        }
    } else if (str_contains($requestUri, "coins/markets?")) {
        $expireTime = 1 * 60 * 60;
    } else if (str_contains($requestUri, "search/trending")) {
        $expireTime = 1 * 60 * 60;
    } else if (str_contains($requestUri, "simple/price")) {
        $expireTime = 5 * 60;
    } else if (str_contains($requestUri, "/ping")) {
        header('Cache-Control: public, max-age=1200');
        $expireTime = 6 * 60 * 60;
    } else if (str_contains($requestUri, "/coins/list")) {
        header('Cache-Control: public, max-age=1200');
        $expireTime = 6 * 60 * 60;
    }

    incStatsValue("requests");
    incStatsValue($_SERVER['REMOTE_ADDR']);

    file_put_contents("requests.log", timePrefix() . $_SERVER['REMOTE_ADDR'] . " - " . $requestUri, FILE_APPEND);

    $coinGeckoUrl = 'https://api.coingecko.com/';

    // Create a unique identifier for the cache based on the request URL
    $cacheKey = md5($requestUri);

    $cacheState = isCached($cacheKey, $expireTime);
    if ($cacheState === 0) { // cache hit and valid
        $cachedResult = getCachedResult($cacheKey, $expireTime);
        incStatsValue("cache " . $requestUri);
        $response = $cachedResult;
        file_put_contents("requests.log", " [cache hit]\n", FILE_APPEND);
    } else { // cache miss or expired
        // Create a stream context with options, including 'http' context
        $context = stream_context_create([
            'http' => [
                'method' => 'GET',
                'header' => 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0', // Set a user agent to avoid issues with some servers
            ],
        ]);
        // Create the full URL for the coingecko server
        $requestUrl = $coinGeckoUrl . $requestUri;
        // Make the request and get the content
        $response = file_get_contents($requestUrl, false, $context);
        // Get the HTTP status code from the response headers
        list($version, $statusCode, $reasonPhrase) = explode(' ', $http_response_header[0], 3);
        $statusCode = (int)$statusCode;

        if ($cacheState === 1) { // cache hit but expired
            if ($statusCode === 429) {
                // Prefer the expired data over 429 response code
                $cachedResult = getCachedResult($cacheKey, 24 * 60 * 60);
                http_response_code(200);
                incStatsValue("cache old " . $requestUri);
                $response = $cachedResult;
                file_put_contents("requests.log", " [cache old]\n", FILE_APPEND);
            } else {
                http_response_code($statusCode);
                if ($statusCode === 200) {
                    // Save the result to the cache
                    saveToCache($cacheKey, $response);
                    incStatsValue("gecko " . $requestUri);
                }
                incStatsValue("gecko http " . $statusCode);
                incStatsValue("gecko http " . $statusCode . " - " . $_SERVER['REMOTE_ADDR']);
                file_put_contents("requests.log", " [expired -> $statusCode]\n", FILE_APPEND);
            }
        } else if ($cacheState === 2) {// cache miss
            http_response_code($statusCode);
            if ($statusCode === 200) {
                // Save the result to the cache
                saveToCache($cacheKey, $response);
                incStatsValue("gecko " . $requestUri);
            }
            incStatsValue("gecko http " . $statusCode);
            incStatsValue("gecko http " . $statusCode . " - " . $_SERVER['REMOTE_ADDR']);
            file_put_contents("requests.log", " [$statusCode]\n", FILE_APPEND);
        }
    }
    echo $response;
}

function timePrefix()
{
    return "" . time() . " | ";
}

function incStatsValue($key)
{
    /*
    $c = getStatsValue($key);
    if ($c !== false) {
        $c++;
    } else {
        $c = 1;
    }
    setStatsValue($key, $c);
    */
}

function setStatsValue($key, $value)
{
    /*
    $statsFile = 'stats.json';
    if (file_exists($statsFile)) {
        $jsonData = file_get_contents($statsFile);
        $data = json_decode($jsonData, true);
        $data[$key] = $value;
        ksort($data);
    } else {
        $data = [$key => $value];
    }
    // Convert the data to JSON format
    $jsonData = json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
    // Write the JSON data to a file
    file_put_contents($statsFile, $jsonData);
    */
}

function getStatsValue($key)
{
    /*
    $statsFile = 'stats.json';
    if (file_exists($statsFile)) {
        $jsonData = file_get_contents($statsFile);
        $data = json_decode($jsonData, true);
        if (array_key_exists($key, $data)) {
            return $data[$key];
        }
    }
    return false;
    */
}

// Function to check cache entry
// ret 0: hit and valid
// ret 1: hit but expired
// ret 2: miss
function isCached($cacheKey, $expireTime) {
    $cacheFile = 'cache/' . $cacheKey;
    if (file_exists($cacheFile)) {
        if ((time() - filemtime($cacheFile)) < $expireTime) {
            return 0;
        } else {
            return 1;
        }
    }
    return 2;
}

// Function to retrieve cached result
function getCachedResult($cacheKey, $expireTime) {
    $cacheFile = 'cache/' . $cacheKey;
    if (file_exists($cacheFile)) {
        if ((time() - filemtime($cacheFile)) < $expireTime) {
            incStatsValue("cache hit");
            // file_put_contents("cache/cache_age.log", timePrefix() . "$cacheKey hit @ age: " . (time() - filemtime($cacheFile)) . ", expires: $expireTime\n", FILE_APPEND);
            return file_get_contents($cacheFile);
        } else {
            // Delete the expired cache file
            incStatsValue("cache expired");
            // file_put_contents("cache/cache_age.log", timePrefix() . "$cacheKey expired, hit @ age: " . (time() - filemtime($cacheFile)) . ", expires: $expireTime\n", FILE_APPEND);
            unlink($cacheFile);
        }
    }
    return false;
}

// Function to save result to cache
function saveToCache($cacheKey, $data) {
    $cacheDir = 'cache';
    if (!is_dir($cacheDir)) {
        mkdir($cacheDir, 0777, true);
    }
    // file_put_contents("cache/cache_age.log", timePrefix() . "$cacheKey created\n", FILE_APPEND);
    $cacheFile = $cacheDir . '/' . $cacheKey;
    file_put_contents($cacheFile, $data);
}

?>