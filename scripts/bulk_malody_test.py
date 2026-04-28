import os
import subprocess
import sys
import time
from concurrent.futures import ProcessPoolExecutor

# 配置
MALODY_FILES_DIR = "/home/xiang/Documents/MusicMapRepo/ma"
TEST_EXE = "./build/bin/MalodyConsistencyTest"
OUTPUT_DIR = "./build/test_output/bulk_malody_test"
REPORT_FILE = "./build/test_output/malody_bulk_report.txt"
MAX_WORKERS = 8

def run_test(file_path):
    file_name = os.path.basename(file_path)
    # 使用与输入文件相同的名字，以防逻辑依赖文件名（虽然Malody通常不依赖）
    output_path = os.path.join(OUTPUT_DIR, file_name)
    
    try:
        result = subprocess.run(
            [TEST_EXE, file_path, output_path],
            capture_output=True,
            text=True,
            timeout=15
        )
        if result.returncode == 0:
            return True, file_path, ""
        else:
            # 提取错误信息中的关键部分
            error_msg = result.stdout + result.stderr
            return False, file_path, error_msg
    except subprocess.TimeoutExpired:
        return False, file_path, "Timeout (15s)"
    except Exception as e:
        return False, file_path, str(e)

def main():
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    print(f"Scanning {MALODY_FILES_DIR} for Malody (.mc) maps...")
    mc_maps = []
    for root, dirs, files in os.walk(MALODY_FILES_DIR):
        for f in files:
            if f.endswith(".mc"):
                full_path = os.path.join(root, f)
                mc_maps.append(full_path)
    
    total_found = len(mc_maps)
    print(f"Found {total_found} Malody maps. Starting tests with {MAX_WORKERS} workers...")

    start_time = time.time()
    results = []
    
    with ProcessPoolExecutor(max_workers=MAX_WORKERS) as executor:
        future_to_path = {executor.submit(run_test, path): path for path in mc_maps}
        
        count = 0
        success_count = 0
        failed_list = []
        
        for future in future_to_path:
            success, path, error = future.result()
            count += 1
            if success:
                success_count += 1
            else:
                failed_list.append((path, error))
            
            if count % 10 == 0 or count == total_found:
                print(f"Progress: {count}/{total_found} (Success: {success_count})", end='\r')

    end_time = time.time()
    duration = end_time - start_time

    # 生成报告
    with open(REPORT_FILE, 'w', encoding='utf-8') as f:
        f.write("=== Malody Bulk Test Report ===\n")
        f.write(f"Timestamp: {time.ctime()}\n")
        f.write(f"Total Maps: {total_found}\n")
        f.write(f"Success: {success_count}\n")
        f.write(f"Failed: {len(failed_list)}\n")
        f.write(f"Pass Rate: {(success_count/total_found)*100:.2f}%\n")
        f.write(f"Duration: {duration:.2f} seconds\n\n")
        
        if failed_list:
            f.write("--- Failed Maps Details ---\n")
            for path, error in failed_list:
                f.write(f"\n[FILE]: {path}\n")
                f.write(f"[ERROR]: {error}\n")
                f.write("-" * 40 + "\n")

    print(f"\n\nFinal Result: {success_count}/{total_found} passed.")
    print(f"Detailed report saved to: {REPORT_FILE}")
    
    if success_count < total_found:
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == "__main__":
    main()
