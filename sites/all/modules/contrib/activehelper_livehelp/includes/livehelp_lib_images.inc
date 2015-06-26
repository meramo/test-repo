<?php

/**
 * Uploads a file to a path destination with the name given.
 * 
 * @param string $path
 * @param string $name
 * @param array $file
 * @param boolean $file_extension Optional.
 * @return string|array 
 */
function activehelper_livehelp__lib_images_upload($path, $name, $file, $file_extension = FALSE) {
  $file_info = pathinfo($file['tmp_name']);

  $tempImage = $path . '/__temp__' . $file_info['basename'];

  @chmod($path, 0777);
  if (!is_writable($path)) {
    @unlink($file['tmp_name']);
    return t('The folder @folder is not writable.', array('@folder' => $path));
  }

  if (!move_uploaded_file($file['tmp_name'], $tempImage)) {
    @unlink($file['tmp_name']);
    return t('The file @file was not able to be moved.', array('@file' => $file['tmp_name']));
  }

  $file['tmp_name'] = $tempImage;
  $size = @getimagesize($file['tmp_name']);
  if ($size === FALSE || !is_array($size)) {
    @unlink($file['tmp_name']);
    return t('The file @file seems not to be a valid image.', array('@file' => $name));
  }

  $extension = array('1' => '.gif', '2' => '.jpg', '3' => '.png', '6' => '.bmp');
  if (isset($extension[$size[2]]))
    $extension = $extension[$size[2]];
  else {
    @unlink($file['tmp_name']);
    return t('The file @file seems not to have a valid image extension (gif, jpg, png or bmp).', $name);
  }

  if (!empty($file_extension) && $file_extension != $extension) {
    @unlink($file['tmp_name']);
    return t('The file @file must be in @extension format.', array('@file' => $name, '@extension' => $file_extension));
  }

  $image = $path . '/' . $name . $extension;
  if (file_exists($image)) {
    @unlink($image);
  }
  if (!rename($file['tmp_name'], $image)) {
    if (file_exists($file['tmp_name'])) {
      @unlink($file['tmp_name']);
    }

    return t('The file @file was not able to be renamed.', array('@file' => $file['tmp_name']));
  }

  @chmod($image, 0644);
  if (file_exists($file['tmp_name'])) {
    @unlink($file['tmp_name']);
  }

  return array(
    'dirname' => $path,
    'basename' => $name . $extension,
    'extension' => drupal_substr($extension, 1),
    'filename' => $name
  );
}

/**
 * Deletes an image file if it exists.
 * 
 * @param string $path
 * @param string $name
 * @return boolean 
 */
function activehelper_livehelp__lib_images_delete($path, $name) {
  $image = $path . '/' . $name;

  if (!file_exists($image) || !is_file($image))
    return t('The file %s does not exist.', $image);

  return @unlink($image);
}

