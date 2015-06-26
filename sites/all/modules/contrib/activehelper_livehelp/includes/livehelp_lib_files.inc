<?php

/**
 * Copy the source folder and its content to destination.
 * 
 * @param string $source
 * @param string $destination 
 */
function activehelper_livehelp__lib_files_duplicate($source, $destination) {
  if (is_dir($source)) {
    @mkdir($destination);
    $dir = dir($source);

    while (FALSE !== ( $file = $dir->read() )) {
      if ($file == '.' || $file == '..')
        continue;

      $path = $source . '/' . $file;

      if (is_dir($path))
        activehelper_livehelp__lib_files_duplicate($path, $destination . '/' . $file);
      else
        copy($path, $destination . '/' . $file);
    }

    $dir->close();
  }
  else
    copy($source, $destination);
}

/**
 * Deletes a source folder and its content.
 * 
 * @param string $source
 * @param boolean $delete_container Optional. False if you only need the source content deleted.
 * @return boolean.
 */
function activehelper_livehelp__lib_files_delete($source, $delete_container = TRUE) {
  if (is_file($source)) {
    return @unlink($source);
  }

  if (!$dh = @opendir($source)) {
    return FALSE;
  }

  while (FALSE !== ( $obj = readdir($dh) )) {
    if ($obj == '.' || $obj == '..')
      continue;

    if (!@unlink($source . '/' . $obj))
      activehelper_livehelp__lib_files_delete($source . '/' . $obj);
  }

  closedir($dh);

  if ($delete_container) {
    @rmdir($source);
  }

  return TRUE;
}

/**
 * Compress a source folder and its content to destination.
 * 
 * @param string $source
 * @param string $destination
 * @return boolean 
 */
function activehelper_livehelp__lib_files_zip($source, $destination) {
  if (extension_loaded('zip') === TRUE) {
    if (file_exists($source) === TRUE) {
      $zip = new ZipArchive();

      if ($zip->open($destination, ZIPARCHIVE::CREATE) === TRUE) {
        $source = realpath($source);

        if (is_dir($source) === TRUE) {
          $files = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($source), RecursiveIteratorIterator::SELF_FIRST);

          $baseSource = basename($source) . '/';
          $zip->addEmptyDir($baseSource);

          foreach ($files as $file) {
            $file = realpath($file);

            if (is_dir($file) === TRUE)
              $zip->addEmptyDir($baseSource . drupal_substr(str_replace($source, '', $file . '/'), 1));
            else if (is_file($file) === TRUE)
              $zip->addFromString($baseSource . drupal_substr(str_replace($source, '', $file), 1), file_get_contents($file));
          }
        }
        else if (is_file($source) === TRUE)
          $zip->addFromString(basename($source), file_get_contents($source));
      }

      return $zip->close();
    }
  }

  return FALSE;
}

function activehelper_livehelp__lib_files_unzip($source, $destination) {
  $zip = new ZipArchive;

  if ($zip->open($source)) {
    $zip->extractTo($destination);
    $zip->close();

    return TRUE;
  }

  return FALSE;
}

